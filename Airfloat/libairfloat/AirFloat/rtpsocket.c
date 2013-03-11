//
//  rtpsocket.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/20/13.
//
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "rtpsocket.h"

struct rtp_socket_info_t {
    
    struct rtp_socket_t* owner;
    socket_p socket;
    bool is_data_socket;
    thread_p thread;
    char* name;
    
};

struct rtp_socket_t {
    char* name;
    struct sockaddr* allowed_remote_end_point;
    struct rtp_socket_info_t** sockets;
    uint32_t sockets_count;
    rtp_socket_data_received_callback received_callback;
    void* received_callback_ctx;
    mutex_p mutex;
};

struct rtp_socket_t* rtp_socket_create(const char* name, struct sockaddr* allowed_remote_end_point) {
    
    struct rtp_socket_t* rs = (struct rtp_socket_t*)malloc(sizeof(struct rtp_socket_t));
    bzero(rs, sizeof(struct rtp_socket_t));
    
    if (allowed_remote_end_point != NULL)
        rs->allowed_remote_end_point = sockaddr_copy(allowed_remote_end_point);
    
    if (name) {
        rs->name = (char*)malloc(strlen(name) + 1);
        strcpy(rs->name, name);
    }
    
    rs->mutex = mutex_create();
    
    return rs;
    
}

void rtp_socket_destroy(struct rtp_socket_t* rs) {
    
    mutex_lock(rs->mutex);
    
    while (rs->sockets_count > 0) {
        socket_close(rs->sockets[0]->socket);
        mutex_unlock(rs->mutex);
        thread_join(rs->sockets[0]->thread);
        mutex_lock(rs->mutex);
    }
    
    mutex_unlock(rs->mutex);
    
    if (rs->allowed_remote_end_point)
        sockaddr_destroy(rs->allowed_remote_end_point);
    if (rs->name)
        free(rs->name);
    
    mutex_destroy(rs->mutex);
    
    free(rs);
    
}

void _rtp_socket_thread_detach(struct rtp_socket_t* rs, const char *name, socket_p socket, bool is_data_socket, void (*fnc)(void *ctx)) {
    
    struct rtp_socket_info_t* info = (struct rtp_socket_info_t*)malloc(sizeof(struct rtp_socket_info_t));
    bzero(info, sizeof(struct rtp_socket_info_t));
    info->owner = rs;
    info->socket = socket;
    info->is_data_socket = is_data_socket;
    
    if (name != NULL && rs->name != NULL) {
        info->name = (char*)malloc(strlen(rs->name) + 4 + strlen(name));
        sprintf(info->name, "%s - %s", rs->name, name);
    }
    
    mutex_lock(rs->mutex);
    
    rs->sockets = (struct rtp_socket_info_t**)realloc(rs->sockets, sizeof(struct rtp_socket_info_t*) * (rs->sockets_count + 1));
    rs->sockets[rs->sockets_count] = info;
    rs->sockets_count++;
    
    mutex_unlock(rs->mutex);
    
    info->thread = thread_create(fnc, info);
    
}

void _rtp_socket_remove_socket(struct rtp_socket_t* rs, struct rtp_socket_info_t* info) {
    
    mutex_lock(rs->mutex);
    
    for (uint32_t i = 0 ; i < rs->sockets_count ; i++)
        if (rs->sockets[i] == info) {
            for (uint32_t a = i + 1 ; a < rs->sockets_count ; a++)
                rs->sockets[a - 1] = rs->sockets[a];
            rs->sockets_count--;
            break;
        }
    
    mutex_unlock(rs->mutex);
    
    socket_destroy(info->socket);
    
    if (info->name != NULL)
        free(info->name);
    
    free(info);
    
}

void _rtp_socket_receive_loop(void* ctx) {
    
    struct rtp_socket_info_t* info = (struct rtp_socket_info_t*)ctx;
    struct rtp_socket_t* rs = info->owner;
    
    if (info->name != NULL)
        thread_set_name(info->name);
    
    size_t offset = 0;
    unsigned char buffer[32768];
    
    for (;;) {
        
        if (offset == 32768)
            break;
        
        int64_t read = socket_receive(info->socket, &buffer[offset], 32768 - offset);
        
        if (read <= 0)
            break;
        
        size_t used = read;
        
        if ((rs->allowed_remote_end_point == NULL || sockaddr_equals_host(socket_get_remote_end_point(info->socket), rs->allowed_remote_end_point)) && rs->received_callback != NULL)
            used = rs->received_callback(rs, info->socket, (const char*)buffer, read + offset, rs->received_callback_ctx);
        
        assert(used <= read + offset);
        
        offset = read + offset - used;
        if (used > 0 && offset > 0)
            memcpy(buffer, &buffer[used], offset);
        
    }
    
    _rtp_socket_remove_socket(rs, info);
    
}

void _rtp_socket_accept_loop(void* ctx) {
    
    struct rtp_socket_info_t* info = (struct rtp_socket_info_t*)ctx;
    struct rtp_socket_t* rs = info->owner;
    
    thread_set_name(info->name);
    
    for (;;) {
        
        socket_p new_socket = socket_accept(info->socket);
        
        if (!new_socket)
            break;
        
        if (rs->allowed_remote_end_point == NULL || sockaddr_equals_host(socket_get_remote_end_point(new_socket), rs->allowed_remote_end_point))
            _rtp_socket_thread_detach(rs, "TCP Connection", new_socket, true, _rtp_socket_receive_loop);
        else {
            socket_close(new_socket);
            socket_destroy(new_socket);
        }
        
    }
    
    _rtp_socket_remove_socket(rs, info);
    
}

bool rtp_socket_setup(struct rtp_socket_t* rs, struct sockaddr* local_end_point) {
    
    socket_p udp_socket = socket_create(true);
    socket_p tcp_socket = socket_create(false);
    
    if (socket_bind(udp_socket, local_end_point) && socket_bind(tcp_socket, local_end_point) && socket_listen(tcp_socket)) {
        _rtp_socket_thread_detach(rs, "UDP Socket", udp_socket, true, _rtp_socket_receive_loop);
        _rtp_socket_thread_detach(rs, "TCP Accept", tcp_socket, false, _rtp_socket_accept_loop);
        return true;
    }
    
    socket_destroy(udp_socket);
    socket_destroy(tcp_socket);
    
    return false;
    
}

void rtp_socket_set_data_received_callback(struct rtp_socket_t* rs, rtp_socket_data_received_callback callback, void* ctx) {
    
    rs->received_callback = callback;
    rs->received_callback_ctx = ctx;
    
}

void rtp_socket_send_to(struct rtp_socket_t* rs, struct sockaddr* dst, const void* buffer, uint32_t size) {
    
    for (uint32_t i = 0 ; i < rs->sockets_count ; i++)
        if (rs->sockets[i]->is_data_socket)
            socket_send_to(rs->sockets[i]->socket, dst, buffer, size);
    
}

uint16_t rtp_socket_get_local_port(rtp_socket_p rs) {
    
    for (uint32_t i = 0 ; i < rs->sockets_count ; i++)
        if (!rs->sockets[i]->is_data_socket)
            return sockaddr_get_port(socket_get_local_end_point(rs->sockets[i]->socket));
    
    return 0;
    
}

//
//  Socket.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#include "log.h"

#include "sockaddr.h"
#include "socket.h"

struct socket_t {
    bool is_udp;
    int socket;
    struct sockaddr* local_end_point;
    struct sockaddr* remote_end_point;
};

socket_p socket_create(bool is_udp) {
    
    struct socket_t* s = (struct socket_t*)malloc(sizeof(struct socket_t));
    bzero(s, sizeof(struct socket_t));
    
    s->is_udp = is_udp;
    s->socket = -1;
    
    return s;
    
}

void socket_destroy(socket_p s) {
    
    if (s->socket > -1)
        socket_close(s);
    
    if (s->local_end_point != NULL)
        sockaddr_destroy(s->local_end_point);
    if (s->remote_end_point != NULL)
        sockaddr_destroy(s->remote_end_point);
    
    free(s);
    
}

bool socket_bind(socket_p s, struct sockaddr* end_point) {
    
    assert(end_point != NULL);
    
    if (s->local_end_point != NULL) {
        sockaddr_destroy(s->local_end_point);
        s->local_end_point = NULL;
    }
    
    struct sockaddr* ep = sockaddr_copy(end_point);
    s->local_end_point = ep;
    
    if (s->socket < 0) {
        
        s->socket = socket(ep->sa_family, (s->is_udp ? SOCK_DGRAM : SOCK_STREAM), (s->is_udp ? IPPROTO_UDP : IPPROTO_TCP));
        
        if (s->socket < 0)
            log_message(LOG_INFO, "Socket creation error: %s", strerror(errno));
        
        if (sockaddr_is_ipv6(ep)) {
            int32_t on = 1;
            setsockopt(s->socket, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
        }
        
    }
    
    if (bind(s->socket, ep, ep->sa_len) == 0)
        return true;
    
    return false;
    
}

bool socket_listen(socket_p s) {
    
    if (listen(s->socket, 5) == 0)
        return true;
    
    return false;
    
}

socket_p socket_accept(socket_p s) {
    
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int new_socket = accept(s->socket, (struct sockaddr*)&client_addr, &addr_len);
    
    if (new_socket >= 0) {
        
        struct socket_t* ret = (struct socket_t*)malloc(sizeof(struct socket_t));
        bzero(ret, sizeof(struct socket_t));
        
        ret->is_udp = false;
        ret->socket = new_socket;
        
        return ret;
        
    }
    
    return NULL;
    
}

int64_t socket_receive(socket_p s, void* buffer, uint32_t size) {

    assert(buffer != NULL && size > 0);
    
    if (s->is_udp) {
        
        struct sockaddr_storage remote_addr;
        socklen_t remote_addr_len = sizeof(struct sockaddr_storage);
        int ret = recvfrom(s->socket, buffer, size, 0, (struct sockaddr*) &remote_addr, &remote_addr_len);
        
        if (s->is_udp) {
            if (s->remote_end_point != NULL) {
                sockaddr_destroy(s->remote_end_point);
                s->remote_end_point = NULL;
            }
            
            s->remote_end_point = sockaddr_copy((struct sockaddr*) &remote_addr);
        }
        
        return ret;
        
    } else
        return recv(s->socket, buffer, size, 0);
    
}

int64_t socket_send(socket_p s, const void* buffer, uint32_t size) {
    
    return send(s->socket, buffer, size, 0);
    
}

int64_t socket_send_to(socket_p s, struct sockaddr* end_point, const void* buffer, uint32_t size) {
    
    assert(end_point != NULL && buffer != NULL && size > 0);
    
    if (!s->is_udp)
        return socket_send(s, buffer, size);
    
    socklen_t len = end_point->sa_len;
    
    assert(end_point->sa_family == s->local_end_point->sa_family);
    
    int64_t ret = sendto(s->socket, buffer, size, 0, (struct sockaddr*) end_point, len);
    
    if (ret < 0)
        log_message(LOG_INFO, "Unable to send (errno: %d - %s)", errno, strerror(errno));
    
    return ret;
    
}

void socket_close(socket_p s) {
    
    close(s->socket);
    s->socket = -1;
    
}

struct sockaddr* socket_get_local_end_point(socket_p s) {
    
    if (s->local_end_point == NULL && s->socket >= 0) {
        
        struct sockaddr_storage* addr = (struct sockaddr_storage*)malloc(sizeof(struct sockaddr_storage));
        socklen_t len = sizeof(struct sockaddr_storage);
        bzero(addr, len);
        if (getsockname(s->socket, (struct sockaddr*)addr, &len) == 0)
            s->local_end_point = (struct sockaddr*)addr;
        
    }
    
    return s->local_end_point;
    
}

struct sockaddr* socket_get_remote_end_point(socket_p s) {
    
    if (s->remote_end_point == NULL && s->socket >= 0) {
        
        struct sockaddr_storage* addr = (struct sockaddr_storage*)malloc(sizeof(struct sockaddr_storage));
        socklen_t len = sizeof(struct sockaddr_storage);
        if (getpeername(s->socket, (struct sockaddr*)addr, &len) == 0)
            s->remote_end_point = (struct sockaddr*)addr;
        
    }
    
    return s->remote_end_point;
    
}

bool socket_is_udp(socket_p s) {
    
    return s->is_udp;
    
}

//
//  Socket.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

#include "log.h"
#include "mutex.h"
#include "thread.h"

#include "sockaddr.h"
#include "socket.h"

struct socket_t {
    char* name;
    bool is_udp;
    int socket;
    bool is_connected;
    mutex_p mutex;
    thread_p accept_thread;
    thread_p receive_thread;
    struct {
        socket_accept_callback accept;
        socket_connected_callback connected;
        socket_connect_failed_callback connect_failed;
        socket_receive_callback receive;
        socket_closed_callback closed;
        struct {
            void* accept;
            void* connected;
            void* connect_failed;
            void* receive;
            void* closed;
        } ctx;
    } callbacks;
    struct sockaddr* local_end_point;
    struct sockaddr* remote_end_point;
};

void _socket_set_loop_name(struct socket_t* s, const char* name) {
    
    if (s->name != NULL) {
        
        size_t len = strlen(s->name) + strlen(name) + 4;
        char t_name[len];
        sprintf(t_name, "%s - %s", s->name, name);
        thread_set_name(t_name);
        
    }
    
}

void _socket_accept_loop(void* ctx) {
    
    struct socket_t* s = (struct socket_t*)ctx;
   
    mutex_lock(s->mutex);
    _socket_set_loop_name(s, "Accept Loop");
    
    s->is_connected = true;
    
    int new_socket_fd = 0;
    do {
        
        struct sockaddr_storage client_addr;
        socklen_t addr_len = sizeof(client_addr);
        mutex_unlock(s->mutex);
        new_socket_fd = accept(s->socket, (struct sockaddr*)&client_addr, &addr_len);
        mutex_lock(s->mutex);
        
        if (new_socket_fd >= 0) {
            
            struct socket_t* new_socket = (struct socket_t*)malloc(sizeof(struct socket_t));
            bzero(new_socket, sizeof(struct socket_t));
            
            new_socket->socket = new_socket_fd;
            new_socket->mutex = mutex_create();
            new_socket->is_connected = true;
            
            bool accept = false;
            if (s->callbacks.accept != NULL) {
                mutex_unlock(s->mutex);
                accept = s->callbacks.accept(s, new_socket, s->callbacks.ctx.accept);
                mutex_lock(s->mutex);
            }
            
            if (!accept)
                socket_destroy(new_socket);
            
        }
        
    } while (new_socket_fd >= 0);
    mutex_unlock(s->mutex);
    
    socket_close(s);
    
}

void _socket_receive_loop(void* ctx) {
    
    struct socket_t* s = (struct socket_t*)ctx;
    
    mutex_lock(s->mutex);
    _socket_set_loop_name(s, "Receive Loop");
    
    s->is_connected = true;
    
    void* buffer = NULL;
    size_t buffer_size = 0;
    size_t write_pos = 0;
    ssize_t processed = 0;
    
    ssize_t read = 0;
    
    do {
        
        if (buffer_size - write_pos < 16384) {
            buffer_size += 16384;
            buffer = realloc(buffer, buffer_size);
        }
        
        if (s->is_udp) {
            
            struct sockaddr_storage remote_addr;
            socklen_t remote_addr_len = sizeof(struct sockaddr_storage);
            mutex_unlock(s->mutex);
            read = recvfrom(s->socket, buffer + write_pos, buffer_size - write_pos, 0, (struct sockaddr*) &remote_addr, &remote_addr_len);
            mutex_lock(s->mutex);
            
            if (s->remote_end_point != NULL) {
                sockaddr_destroy(s->remote_end_point);
                s->remote_end_point = NULL;
            }
            
            s->remote_end_point = sockaddr_copy((struct sockaddr*) &remote_addr);
            
        } else {
            mutex_unlock(s->mutex);
            read = recv(s->socket, buffer + write_pos, buffer_size - write_pos, 0);
            mutex_lock(s->mutex);
        }
        
        if (read > 0) {
            
            write_pos += read;
            processed = write_pos;
            
            if (s->callbacks.receive != NULL) {
                mutex_unlock(s->mutex);
                processed = s->callbacks.receive(s, buffer, write_pos, s->remote_end_point, s->callbacks.ctx.receive);
                mutex_lock(s->mutex);
            }
            
            if (processed > 0) {
                memcpy(buffer, buffer + processed, write_pos - processed);
                write_pos -= processed;
            }
            
        }
        
    } while (read > 0 && processed >= 0);
    
    if (buffer != NULL)
        free(buffer);
    
    mutex_unlock(s->mutex);
    
    socket_close(s);
    
}

void _socket_connect(void* ctx) {
    
    struct socket_t* s = (struct socket_t*)ctx;
    
    if (!connect(s->socket, s->remote_end_point, s->remote_end_point->sa_len)) {
        
        if (s->callbacks.connected != NULL)
            s->callbacks.connected(s, s->callbacks.ctx.connected);
        
        _socket_receive_loop(ctx);
        
    } else {
        
        log_message(LOG_ERROR, "Unable to connect (%s)", strerror(errno));
        
        if (s->callbacks.connect_failed != NULL)
            s->callbacks.connect_failed(s, s->callbacks.ctx.connect_failed);
        
    }
    
}

struct socket_t* socket_create(const char* name, bool is_udp) {
    
    struct socket_t* s = (struct socket_t*)malloc(sizeof(struct socket_t));
    bzero(s, sizeof(struct socket_t));
    
    if (name != NULL) {
        s->name = (char*)malloc(strlen(name) + 1);
        strcpy(s->name, name);
    }
    
    s->is_udp = is_udp;
    s->socket = -1;
    s->mutex = mutex_create();
    
    return s;
    
}

void socket_destroy(struct socket_t* s) {
    
    socket_close(s);
    
    mutex_lock(s->mutex);
    
    if (s->local_end_point != NULL) {
        sockaddr_destroy(s->local_end_point);
        s->local_end_point = NULL;
    }
    
    if (s->remote_end_point != NULL) {
        sockaddr_destroy(s->remote_end_point);
        s->remote_end_point = NULL;
    }
    
    if (s->name != NULL) {
        free(s->name);
        s->name = NULL;
    }
    
    mutex_unlock(s->mutex);
    
    mutex_destroy(s->mutex);
    
    free(s);
    
}

bool socket_bind(struct socket_t* s, struct sockaddr* end_point) {
    
    assert(end_point != NULL);
    
    if (s->local_end_point != NULL) {
        sockaddr_destroy(s->local_end_point);
        s->local_end_point = NULL;
    }
    
    struct sockaddr* ep = sockaddr_copy(end_point);
    s->local_end_point = ep;
    
    if (s->socket < 0) {
        
        s->socket = socket(ep->sa_family, (s->is_udp ? SOCK_DGRAM : SOCK_STREAM), (s->is_udp ? IPPROTO_UDP : IPPROTO_TCP));
        
        if (s->socket < 0) {
            log_message(LOG_ERROR, "Socket creation error: %s", strerror(errno));
            return false;
        }
        
        if (sockaddr_is_ipv6(ep)) {
            int32_t on = 1;
            setsockopt(s->socket, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
        }
        
    }
    
    if (!s->is_udp) {
        int so_reuseaddr = 1;
        setsockopt(s->socket, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(int));
    }
    
    if (bind(s->socket, ep, ep->sa_len) == 0)
        return true;
    
    
    return false;
    
}

void socket_connect(struct socket_t* s, struct sockaddr* end_point) {
    
    if (!s->is_connected && !s->is_udp) {
        
        if (s->socket <= 0)
            s->socket = socket(end_point->sa_family, SOCK_STREAM, IPPROTO_TCP);
        
        if (s->socket <= 0)
            log_message(LOG_ERROR, "Socket creation error: %s", strerror(errno));
        
        if (s->remote_end_point != NULL)
            sockaddr_destroy(s->remote_end_point);
        
        s->remote_end_point = sockaddr_copy(end_point);
        
        s->receive_thread = thread_create(_socket_connect, s);
        
    }
    
}

void socket_set_accept_callback(struct socket_t* s, socket_accept_callback callback, void* ctx) {
    
    s->callbacks.accept = callback;
    s->callbacks.ctx.accept = ctx;
    
    if (!s->is_udp && s->accept_thread == NULL) {
        
        if (listen(s->socket, 5) == 0)
            s->accept_thread = thread_create(_socket_accept_loop, s);
        
    }
    
}

void socket_set_connected_callback(struct socket_t* s, socket_connected_callback callback, void* ctx) {
    
    s->callbacks.connected = callback;
    s->callbacks.ctx.connected = ctx;
    
}

void socket_set_connect_failed_callback(struct socket_t* s, socket_connect_failed_callback callback, void* ctx) {
    
    s->callbacks.connect_failed = callback;
    s->callbacks.ctx.connect_failed = ctx;
    
}

void socket_set_receive_callback(struct socket_t* s, socket_receive_callback callback, void* ctx) {
    
    s->callbacks.receive = callback;
    s->callbacks.ctx.receive = ctx;
    
    if (s->receive_thread == NULL && (s->is_udp || s->is_connected))
        s->receive_thread = thread_create(_socket_receive_loop, s);
    
}

void socket_set_closed_callback(struct socket_t* s, socket_closed_callback callback, void* ctx) {
    
    s->callbacks.closed = callback;
    s->callbacks.ctx.closed = ctx;
    
}

ssize_t socket_send(struct socket_t* s, const void* buffer, size_t size) {
    
    ssize_t ret = 0;
    
    mutex_lock(s->mutex);
    
    if (s->is_connected)
        ret = send(s->socket, buffer, size, 0);
    
    mutex_unlock(s->mutex);
    
    return ret;
    
}

ssize_t socket_send_to(struct socket_t* s, struct sockaddr* end_point, const void* buffer, size_t size) {
    
    assert(end_point != NULL && buffer != NULL && size > 0);
    
    size_t ret = 0;
    
    mutex_lock(s->mutex);
    
    if (!s->is_udp) {
        mutex_unlock(s->mutex);
        return socket_send(s, buffer, size);
    }
    
    socklen_t len = end_point->sa_len;
    
    assert(end_point->sa_family == s->local_end_point->sa_family);
    
    if (s->is_connected || (s->is_udp && s->socket > -1))
        ret = sendto(s->socket, buffer, size, 0, (struct sockaddr*) end_point, len);
    
    mutex_unlock(s->mutex);
    
    return ret;
    
}

void socket_close(struct socket_t* s) {
    
    mutex_lock(s->mutex);
    
    if (s->is_connected) {
        
        s->is_connected = false;
        close(s->socket);
        s->socket = -1;

        if (s->accept_thread != NULL) {
            mutex_unlock(s->mutex);
            thread_destroy(s->accept_thread);
            mutex_lock(s->mutex);
            s->accept_thread = NULL;
        }
        
        if (s->receive_thread != NULL) {
            mutex_unlock(s->mutex);
            thread_destroy(s->receive_thread);
            mutex_lock(s->mutex);
            s->receive_thread = NULL;
        }
        
        if (s->callbacks.closed) {
            mutex_unlock(s->mutex);
            s->callbacks.closed(s, s->callbacks.ctx.closed);
            mutex_lock(s->mutex);
        }
        
    }
    
    mutex_unlock(s->mutex);
    
}

struct sockaddr* socket_get_local_end_point(struct socket_t* s) {
    
    mutex_lock(s->mutex);
    
    if (s->local_end_point == NULL && s->socket >= 0) {
        
        struct sockaddr_storage* addr = (struct sockaddr_storage*)malloc(sizeof(struct sockaddr_storage));
        socklen_t len = sizeof(struct sockaddr_storage);
        bzero(addr, len);
        if (getsockname(s->socket, (struct sockaddr*)addr, &len) == 0)
            s->local_end_point = (struct sockaddr*)addr;
        else
            free(addr);
        
    }
    
    struct sockaddr* ret = s->local_end_point;
    
    mutex_unlock(s->mutex);
    
    return ret;
    
}

struct sockaddr* socket_get_remote_end_point(struct socket_t* s) {
    
    mutex_lock(s->mutex);
    
    if (s->remote_end_point == NULL && s->socket >= 0) {
        
        struct sockaddr_storage* addr = (struct sockaddr_storage*)malloc(sizeof(struct sockaddr_storage));
        socklen_t len = sizeof(struct sockaddr_storage);
        if (getpeername(s->socket, (struct sockaddr*)addr, &len) == 0)
            s->remote_end_point = (struct sockaddr*)addr;
        else
            free(addr);
        
    }
    
    struct sockaddr* ret = s->remote_end_point;
    
    mutex_unlock(s->mutex);
    
    return ret;
    
}

bool socket_is_udp(struct socket_t* s) {
    
    return s->is_udp;
    
}

bool socket_is_connected(struct socket_t* s) {
    
    return s->is_connected;
    
}

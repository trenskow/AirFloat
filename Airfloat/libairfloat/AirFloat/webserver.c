//
//  webserver.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "mutex.h"
#include "sockaddr.h"
#include "socket.h"
#include "webconnection.h"
#include "webserver.h"

web_connection_p web_connection_create(socket_p socket, web_server_p server);
void web_connection_destroy(web_connection_p connection);

struct web_server_t {
    socket_p socket_ipv4;
    socket_p socket_ipv6;
    sockaddr_type socket_types;
    bool is_running;
    thread_p accept_loop_ipv4;
    thread_p accept_loop_ipv6;
    web_server_accept_callback accept_callback;
    void* accept_callback_ctx;
    web_connection_p* connections;
    uint32_t connection_count;
    mutex_p mutex;
};

struct web_server_thread_data_t {
    struct web_server_t* ws;
    socket_p socket;
};

struct web_server_t* web_server_create(sockaddr_type socket_types) {
    
    struct web_server_t* ws = (struct web_server_t*)malloc(sizeof(struct web_server_t));
    bzero(ws, sizeof(struct web_server_t));
    
    ws->socket_types = socket_types;
    ws->mutex = mutex_create();
    
    return ws;
    
}

void web_server_destroy(struct web_server_t* ws) {
    
    web_server_stop(ws);
    
    mutex_destroy(ws->mutex);
    
    free(ws);
    
}

socket_p _web_server_bind(struct web_server_t* ws, uint16_t port, sockaddr_type socket_type) {
    
    socket_p socket = NULL;
    
    if (ws->socket_types & socket_type) {
        
        socket = socket_create(false);
        struct sockaddr* endPoint = sockaddr_create(NULL, port, socket_type, 0);
        
        bool ret = socket_bind(socket, endPoint);
        if (ret)
            ret = (ret && socket_listen(socket));
        
        sockaddr_destroy(endPoint);
        
        if (!ret) {
            socket_destroy(socket);
            socket = NULL;
        }
        
    }
    
    return socket;
    
}

void _web_server_accept_loop(void* ctx) {
    
    thread_set_name("Server socket");
    
    struct web_server_thread_data_t* thread_data = (struct web_server_thread_data_t*)ctx;
    
    struct web_server_t* ws = thread_data->ws;
    socket_p socket = thread_data->socket;
    
    free(thread_data);
    
    for (;;) {
        
        socket_p new_socket = socket_accept(socket);
        
        if (new_socket == NULL)
            break;
        
        mutex_lock(ws->mutex);
        
        web_connection_p new_connection = web_connection_create(new_socket, ws);
        
        bool shouldLive = (ws->accept_callback != NULL && ws->accept_callback(ws, new_connection, ws->accept_callback_ctx));
        
        if (!shouldLive)
            web_connection_destroy(new_connection);
        else {
            
            ws->connections = (web_connection_p*)realloc(ws->connections, sizeof(web_connection_p) * (ws->connection_count + 1));
            ws->connections[ws->connection_count] = new_connection;
            ws->connection_count++;
            
            web_connection_take_off(new_connection);
            
        }
        
        mutex_unlock(ws->mutex);
        
    }
    
}

void _web_server_accept_loop_start(struct web_server_t* ws, thread_p* thread, socket_p socket) {
    
    struct web_server_thread_data_t* thread_data = (struct web_server_thread_data_t*) malloc(sizeof(struct web_server_thread_data_t));
    
    thread_data->ws = ws;
    thread_data->socket = socket;
    
    *thread = thread_create(_web_server_accept_loop, thread_data);
    
}

bool web_server_start(struct web_server_t* ws, uint16_t port) {
    
    mutex_lock(ws->mutex);
    
    if (!ws->is_running) {
        
        log_message(LOG_INFO, "Trying port %d", port);
        
        ws->socket_ipv4 = _web_server_bind(ws, port, sockaddr_type_inet_4);
        ws->socket_ipv6 = _web_server_bind(ws, port, sockaddr_type_inet_6);
        
        if (((ws->socket_types & sockaddr_type_inet_4) == 0 || ws->socket_ipv4 != NULL) && ((ws->socket_types & sockaddr_type_inet_6) == 0 || ws->socket_ipv6 != NULL)) {
            
            if ((ws->socket_types & sockaddr_type_inet_4) != 0)
                _web_server_accept_loop_start(ws, &ws->accept_loop_ipv4, ws->socket_ipv4);
            if ((ws->socket_types & sockaddr_type_inet_6) != 0)
                _web_server_accept_loop_start(ws, &ws->accept_loop_ipv6, ws->socket_ipv6);
            
            ws->is_running = true;
            
            mutex_unlock(ws->mutex);
            
            return true;
            
        }
        
        if (ws->socket_ipv4 != NULL)
            socket_destroy(ws->socket_ipv4);
        if (ws->socket_ipv6 != NULL)
            socket_destroy(ws->socket_ipv6);
        
        log_message(LOG_INFO, "Server started.");
        
    } else
        log_message(LOG_ERROR, "Cannot start: Server is already running");
    
    mutex_unlock(ws->mutex);
    
    return false;
    
}

bool web_server_is_running(struct web_server_t* ws) {
    
    mutex_lock(ws->mutex);
    bool ret = ws->is_running;
    mutex_unlock(ws->mutex);
    
    return ret;
    
}

void web_server_stop(struct web_server_t* ws) {
    
    mutex_lock(ws->mutex);
    
    if (ws->is_running) {
        
        if (ws->socket_ipv4 != NULL)
            socket_close(ws->socket_ipv4);
        if (ws->socket_ipv6 != NULL)
            socket_close(ws->socket_ipv6);
        
        // Await accept threads exit
        if ((ws->socket_types & sockaddr_type_inet_4) != 0)
            thread_join(ws->accept_loop_ipv4);
        if ((ws->socket_types & sockaddr_type_inet_6) != 0)
            thread_join(ws->accept_loop_ipv6);
        
        if (ws->socket_ipv4 != NULL) {
            socket_destroy(ws->socket_ipv4);
            ws->socket_ipv4 = NULL;
        }
        if (ws->socket_ipv6 != NULL) {
            socket_destroy(ws->socket_ipv6);
            ws->socket_ipv6 = NULL;
        }
        
        ws->is_running = false;
        
        while (ws->connection_count > 0)
            web_connection_destroy(ws->connections[0]);
        
        log_message(LOG_INFO, "Server stopped");
        
    } else
        log_message(LOG_ERROR, "Cannot stop: Server is not running");
    
    mutex_unlock(ws->mutex);
    
}

uint32_t web_server_get_connection_count(struct web_server_t* ws) {
    
    mutex_lock(ws->mutex);
    uint32_t ret = ws->connection_count;
    mutex_unlock(ws->mutex);
    
    return ret;
    
}

struct sockaddr* web_server_get_local_end_point(struct web_server_t* ws, sockaddr_type socket_type) {
    
    return socket_get_local_end_point((ws->socket_types & sockaddr_type_inet_4 ? ws->socket_ipv4 : ws->socket_ipv6));
    
    return NULL;

}

void web_server_set_accept_callback(struct web_server_t* ws, web_server_accept_callback accept_callback, void* ctx) {
    
    mutex_lock(ws->mutex);
    ws->accept_callback = accept_callback;
    ws->accept_callback_ctx = ctx;
    mutex_unlock(ws->mutex);
    
}

void web_server_connection_closed(web_server_p ws, web_connection_p wc) {
    
    mutex_lock(ws->mutex);
    
    uint32_t previous_connection_count = ws->connection_count;
    
    for (uint32_t i = 0 ; i < ws->connection_count ; i++)
        if (ws->connections[i] == wc) {
            web_connection_destroy(wc);
            for (uint32_t x = i ; x < ws->connection_count - 1 ; x++)
                ws->connections[x] = ws->connections[x + 1];
            ws->connection_count--;
            break;
        }
    
    if (previous_connection_count == ws->connection_count)
        log_message(LOG_ERROR, "Server knows nothing about connection %p", wc);
    
    mutex_unlock(ws->mutex);
    
}

//
//  webserver.c
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "mutex.h"
#include "endpoint.h"
#include "socket.h"
#include "webserverconnection.h"
#include "webserver.h"

struct web_server_connection_t {
    web_server_connection_p web_connection;
    socket_p socket;
};

struct web_server_t {
    object_p object;
    socket_p socket_ipv4;
    socket_p socket_ipv6;
    endpoint_type endpoint_types;
    bool is_running;
    web_server_accept_callback accept_callback;
    void* accept_callback_ctx;
    struct web_server_connection_t* connections;
    uint32_t connection_count;
    mutex_p mutex;
};

void _web_server_socket_closed(socket_p socket, void* ctx) {
    
    struct web_server_t* ws = (struct web_server_t*)ctx;
    
    mutex_lock(ws->mutex);
    
    for (uint32_t i = 0 ; i < ws->connection_count ; i++)
        if (ws->connections[i].socket == socket) {
            
            object_release(ws->connections[i].web_connection);
            object_release(ws->connections[i].socket);
            
            for (uint32_t x = i ; x < ws->connection_count - 1 ; x++) {
                ws->connections[x] = ws->connections[x + 1];
            }
            
            ws->connection_count--;
            
            break;
            
        }
    
    mutex_unlock(ws->mutex);
    
}

void _web_server_destroy(void* object) {
    
    struct web_server_t* ws = (struct web_server_t*)object;
    
    web_server_stop(ws);
    
    mutex_destroy(ws->mutex);
    
}

struct web_server_t* web_server_create(endpoint_type endpoint_types) {
    
    struct web_server_t* ws = (struct web_server_t*)object_create(sizeof(struct web_server_t), _web_server_destroy);
    bzero(ws, sizeof(struct web_server_t));
    
    ws->endpoint_types = endpoint_types;
    ws->mutex = mutex_create();
    
    return ws;
    
}

socket_p _web_server_bind(struct web_server_t* ws, uint16_t port, endpoint_type endpoint_type) {
    
    socket_p socket = NULL;
    
    if (ws->endpoint_types & endpoint_type) {
        
        socket = socket_create("Web server", false);
        endpoint_p endpoint = endpoint_create(NULL, port, endpoint_type, 0);
        
        bool ret = socket_bind(socket, endpoint);
        
        object_release(endpoint);
        
        if (!ret) {
            object_release(socket);
            socket = NULL;
        }
        
    }
    
    return socket;
    
}

bool _web_server_socket_accept_callback(socket_p socket, socket_p new_socket, void* ctx) {
    
    struct web_server_t* ws = (struct web_server_t*)ctx;
    
    if (new_socket != NULL) {
        
        mutex_lock(ws->mutex);
        
        web_server_connection_p new_web_connection = web_server_connection_create(new_socket, ws);
        
        bool should_live = false;
        if (ws->accept_callback) {
            mutex_unlock(ws->mutex);
            should_live = ws->accept_callback(ws, new_web_connection, ws->accept_callback_ctx);
            mutex_lock(ws->mutex);
        }
        
        if (should_live) {
            
            socket_set_closed_callback(new_socket, _web_server_socket_closed, ws);
            
            struct web_server_connection_t new_connection = {
                (web_server_connection_p)object_retain(new_web_connection),
                (socket_p)object_retain(new_socket)
            };
            
            ws->connections = (struct web_server_connection_t*)realloc(ws->connections, sizeof(struct web_server_connection_t) * (ws->connection_count + 1));
            ws->connections[ws->connection_count] = new_connection;
            ws->connection_count++;
            
            web_server_connection_take_off(new_web_connection);
            
        }
        
        object_release(new_web_connection);
        
        mutex_unlock(ws->mutex);
        
        return should_live;
        
    }
    
    return false;
    
}

bool web_server_start(struct web_server_t* ws, uint16_t port) {
    
    mutex_lock(ws->mutex);
    
    if (!ws->is_running) {
        
        log_message(LOG_INFO, "Trying port %d", port);
        
        ws->socket_ipv4 = _web_server_bind(ws, port, endpoint_type_inet_4);
        ws->socket_ipv6 = _web_server_bind(ws, port, endpoint_type_inet_6);
        
        if (((ws->endpoint_types & endpoint_type_inet_4) == 0 || ws->socket_ipv4 != NULL) && ((ws->endpoint_types & endpoint_type_inet_6) == 0 || ws->socket_ipv6 != NULL)) {
            
            if ((ws->endpoint_types & endpoint_type_inet_4) != 0) {
                socket_set_accept_callback(ws->socket_ipv4, _web_server_socket_accept_callback, ws);
            }
            if ((ws->endpoint_types & endpoint_type_inet_6) != 0) {
                socket_set_accept_callback(ws->socket_ipv6, _web_server_socket_accept_callback, ws);
            }
            
            ws->is_running = true;
            
            mutex_unlock(ws->mutex);
            
            return true;
            
        }
        
        if (ws->socket_ipv4 != NULL) {
            object_release(ws->socket_ipv4);
        }
        
        if (ws->socket_ipv6 != NULL) {
            object_release(ws->socket_ipv6);
        }
        
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
        
        ws->is_running = false;
        
        if (ws->socket_ipv4 != NULL) {
            object_release(ws->socket_ipv4);
            ws->socket_ipv4 = NULL;
        }
        if (ws->socket_ipv6 != NULL) {
            object_release(ws->socket_ipv6);
            ws->socket_ipv6 = NULL;
        }
        
        while (ws->connection_count > 0) {
            mutex_unlock(ws->mutex);
            object_release(ws->connections[0].socket);
            object_release(ws->connections[0].web_connection);
            mutex_lock(ws->mutex);
        }
        
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

endpoint_p web_server_get_local_endpoint(struct web_server_t* ws, endpoint_type endpoint_type) {
    
    return socket_get_local_endpoint((endpoint_type & endpoint_type_inet_4 ? ws->socket_ipv4 : ws->socket_ipv6));
    
    return NULL;

}

void web_server_set_accept_callback(struct web_server_t* ws, web_server_accept_callback accept_callback, void* ctx) {
    
    mutex_lock(ws->mutex);
    ws->accept_callback = accept_callback;
    ws->accept_callback_ctx = ctx;
    mutex_unlock(ws->mutex);
    
}

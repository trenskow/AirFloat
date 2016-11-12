//
//  webserverconnection.c
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "log.h"
#include "mutex.h"
#include "socket.h"
#include "sockaddr.h"
#include "webtools.h"
#include "webheaders.h"
#include "webrequest.h"
#include "webserverconnection.h"

#define MAX(x,y) (x > y ? x : y)

struct web_server_connection_t {
    mutex_p mutex;
    bool is_connected;
    bool has_taken_off;
    socket_p socket;
    web_server_p server;
    web_server_connection_request_callback request_callback;
    void* request_callback_ctx;
    web_server_connection_closed_callback closed_callback;
    void* closed_callback_ctx;
};

ssize_t _web_server_connection_socket_recieve_callback(socket_p socket, const void* data, size_t data_size, struct sockaddr* remote_end_point, void* ctx) {
    
    struct web_server_connection_t* wc = (struct web_server_connection_t*)ctx;
    
    mutex_lock(wc->mutex);
    
    ssize_t ret = 0;
    
    web_request_p request = web_request_create();
    
    if ((ret = web_request_parse(request, data, data_size)) > 0) {
        
        mutex_unlock(wc->mutex);
        
        if (wc->request_callback != NULL)
            wc->request_callback(wc, request, wc->request_callback_ctx);
        
    } else
        mutex_unlock(wc->mutex);
    
    web_request_destroy(request);
    
    return ret;
        
}

struct web_server_connection_t* web_server_connection_create(socket_p socket, web_server_p server) {
    
    struct web_server_connection_t* wc = (struct web_server_connection_t*)malloc(sizeof(struct web_server_connection_t));
    bzero(wc, sizeof(struct web_server_connection_t));
    
    wc->socket = socket;
    wc->server = server;
    
    wc->is_connected = false;
    
    wc->request_callback = NULL;
    wc->closed_callback = NULL;
    wc->closed_callback_ctx = wc->request_callback_ctx = NULL;
    
    wc->mutex = mutex_create();
    
    return wc;
    
}

void web_server_connection_destroy(struct web_server_connection_t* wc) {
    
    web_server_connection_close(wc);
    
    mutex_destroy(wc->mutex);
    
    free(wc);
    
}

void web_server_connection_set_request_callback(struct web_server_connection_t* wc, web_server_connection_request_callback request_callback, void* ctx) {
    
    mutex_lock(wc->mutex);
    wc->request_callback = request_callback;
    wc->request_callback_ctx = ctx;
    mutex_unlock(wc->mutex);
    
}

void web_server_connection_set_closed_callback(struct web_server_connection_t* wc, web_server_connection_closed_callback closed_callback, void* ctx) {
    
    mutex_lock(wc->mutex);
    wc->closed_callback = closed_callback;
    wc->closed_callback_ctx = ctx;
    mutex_unlock(wc->mutex);
    
}

void web_server_connection_send_response(web_server_connection_p wc, web_response_p response, const char* protocol, bool close_after_send) {
    
    size_t content_length = web_response_get_content(response, NULL, 0);
    
    if (content_length > 0)
        web_headers_set_value(web_response_get_headers(response), "Content-Length", "%d", content_length);
    
    size_t response_length = web_response_write(response, protocol, NULL, 0);
    
    char buffer[content_length + response_length];
    
    web_response_write(response, protocol, buffer, response_length);
    web_response_get_content(response, buffer + response_length, content_length);
    
    socket_send(wc->socket, buffer, content_length + response_length);
    
    log_data(LOG_COMMUNICATION, buffer, content_length + response_length);
    
    if (close_after_send)
        socket_close(wc->socket);
    
}

bool web_server_connection_is_connected(struct web_server_connection_t* wc) {
    
    mutex_lock(wc->mutex);
    bool ret = wc->is_connected;
    mutex_unlock(wc->mutex);
    
    return ret;
    
}

void web_server_connection_take_off(struct web_server_connection_t* wc) {
    
    mutex_lock(wc->mutex);
    
    wc->has_taken_off = wc->is_connected = true;
    
    socket_set_receive_callback(wc->socket, _web_server_connection_socket_recieve_callback, wc);
    
    mutex_unlock(wc->mutex);
    
    const char *ip = sockaddr_get_host(socket_get_remote_end_point(wc->socket));
    
    log_message(LOG_INFO, "RAOPConnection (%p) took over connection from %s:%d", wc, ip, sockaddr_get_port(socket_get_remote_end_point(wc->socket)));
    
}

void web_server_connection_close(struct web_server_connection_t* wc) {
    
    mutex_lock(wc->mutex);
    
    if (wc->is_connected) {
        
        log_message(LOG_INFO, "Client disconnected");
        
        wc->is_connected = false;
        
        socket_close(wc->socket);
        
        mutex_unlock(wc->mutex);
        
        if (wc->closed_callback != NULL)
            wc->closed_callback(wc, wc->closed_callback_ctx);
        
        mutex_lock(wc->mutex);
        
    }
    
    mutex_unlock(wc->mutex);
    
}

struct sockaddr* web_server_connection_get_local_end_point(struct web_server_connection_t* wc) {
    
    return socket_get_local_end_point(wc->socket);
    
}

struct sockaddr* web_server_connection_get_remote_end_point(struct web_server_connection_t* wc) {
    
    return socket_get_remote_end_point(wc->socket);
    
}

const char* web_server_connection_get_host(struct web_server_connection_t* wc) {
    
    return sockaddr_get_host(socket_get_remote_end_point(wc->socket));
    
}

uint16_t web_server_connection_get_port(struct web_server_connection_t* wc) {
    
    return sockaddr_get_port(socket_get_remote_end_point(wc->socket));
    
}

//
//  webclientconnection.c
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

#include "log.h"
#include "mutex.h"
#include "socket.h"

#include "webclientconnection.h"

struct web_client_connection_t {
    socket_p socket;
    mutex_p mutex;
    web_request_p* requests;
    uint32_t requests_count;
    struct {
        web_client_connection_connected_callback connected;
        web_client_connection_connect_failed_callback connect_failed;
        web_client_connection_response_received_callback response_received;
        web_client_connection_disconnected_callback disconnected;
        struct {
            void* connected;
            void* connect_failed;
            void* response_received;
            void* disconnected;
        } ctx;
    } callbacks;
};

void _web_client_connection_send_next_request(struct web_client_connection_t* wc) {
    
    mutex_lock(wc->mutex);
    
    if (web_client_connection_is_connected(wc) && wc->requests_count > 0) {
        
        size_t request_len = web_request_write(wc->requests[0], NULL, 0);
        char* data[request_len];
        web_request_write(wc->requests[0], data, request_len);
        
        log_data(LOG_COMMUNICATION, data, request_len);
        
        socket_send(wc->socket, data, request_len);

    }
    
    mutex_unlock(wc->mutex);
    
}

void _web_client_connection_socket_connected_callback(socket_p socket, void* ctx) {
    
    struct web_client_connection_t* wc = (struct web_client_connection_t*)ctx;
    
    if (wc->callbacks.connected != NULL)
        wc->callbacks.connected(wc, wc->callbacks.ctx.connected);
    
}

void _web_client_connection_socket_connect_failed_callback(socket_p socket, void* ctx) {
    
    struct web_client_connection_t* wc = (struct web_client_connection_t*)ctx;
    
    if (wc->callbacks.connect_failed)
        wc->callbacks.connect_failed(wc, wc->callbacks.ctx.connect_failed);
    
}

ssize_t _web_client_connection_socket_receive_callback(socket_p socket, const void* data, size_t data_size, struct sockaddr* remote_end_point, void* ctx) {
    
    struct web_client_connection_t* wc = (struct web_client_connection_t*)ctx;
    
    web_response_p response = web_response_create();
    
    ssize_t ret = 0;
    if ((ret = web_response_parse(response, data, data_size)) > 0) {
        
        mutex_lock(wc->mutex);
        
        if (wc->callbacks.response_received != NULL) {
            mutex_unlock(wc->mutex);
            wc->callbacks.response_received(wc, wc->requests[0], response, wc->callbacks.ctx.response_received);
            mutex_lock(wc->mutex);
        }
        
        web_request_destroy(wc->requests[0]);
        
        for (uint32_t i = 0 ; i < wc->requests_count - 1 ; i++)
            wc->requests[i] = wc->requests[i + 1];
        
        wc->requests_count--;
        
        mutex_unlock(wc->mutex);
        
        _web_client_connection_send_next_request(wc);
        
    }
    
    web_response_destroy(response);
    
    return ret;
    
}

void _web_connection_socket_closed_callback(socket_p socket, void* ctx) {
    
    struct web_client_connection_t* wc = (struct web_client_connection_t*)ctx;
    
    if (wc->callbacks.disconnected != NULL)
        wc->callbacks.disconnected(wc, wc->callbacks.ctx.disconnected);
    
}

struct web_client_connection_t* web_client_connection_create() {
    
    struct web_client_connection_t* wc = (struct web_client_connection_t*)malloc(sizeof(struct web_client_connection_t));
    bzero(wc, sizeof(struct web_client_connection_t));
    
    wc->mutex = mutex_create();
    
    return wc;
    
}

void web_client_connection_destroy(struct web_client_connection_t* wc) {
    
    if (wc->socket != NULL) {
        socket_destroy(wc->socket);
        wc->socket = NULL;
    }
    
    mutex_destroy(wc->mutex);
    
    free(wc);
    
}

void web_client_connection_set_connected_callback(struct web_client_connection_t* wc, web_client_connection_connected_callback callback, void* ctx) {
    
    wc->callbacks.connected = callback;
    wc->callbacks.ctx.connected = ctx;
    
}

void web_client_connection_set_connect_failed_callback(struct web_client_connection_t* wc, web_client_connection_connect_failed_callback callback, void* ctx) {
    
    wc->callbacks.connect_failed = callback;
    wc->callbacks.ctx.connect_failed = ctx;
    
}

void web_client_connection_set_response_received_callback(struct web_client_connection_t* wc, web_client_connection_response_received_callback callback, void* ctx) {
    
    wc->callbacks.response_received = callback;
    wc->callbacks.ctx.response_received = ctx;
    
}

void web_client_connection_set_disconneced_callback(struct web_client_connection_t* wc, web_client_connection_disconnected_callback callback, void* ctx) {
    
    wc->callbacks.disconnected = callback;
    wc->callbacks.ctx.disconnected = ctx;
    
}

void web_client_connection_connect(struct web_client_connection_t* wc, struct sockaddr* end_point) {
    
    if (wc->socket == NULL) {
        
        wc->socket = socket_create("Web connection", false);
        
        socket_set_connected_callback(wc->socket, _web_client_connection_socket_connected_callback, wc);
        socket_set_connect_failed_callback(wc->socket, _web_client_connection_socket_connect_failed_callback, wc);
        socket_set_receive_callback(wc->socket, _web_client_connection_socket_receive_callback, wc);
        socket_set_closed_callback(wc->socket, _web_connection_socket_closed_callback, wc);
        
        socket_connect(wc->socket, end_point);
        
    }
    
}

bool web_client_connection_is_connected(struct web_client_connection_t* wc) {
    
    return (wc->socket != NULL && socket_is_connected(wc->socket));
    
}

void web_client_connection_send_request(web_client_connection_p wc, web_request_p request) {
    
    if (web_client_connection_is_connected(wc)) {
        
        mutex_lock(wc->mutex);
        
        wc->requests = (web_request_p*)realloc(wc->requests, sizeof(web_request_p) * (wc->requests_count + 1));
        wc->requests[wc->requests_count] = web_request_copy(request);
        wc->requests_count++;
        
        bool send = (wc->requests_count == 1);
        
        mutex_unlock(wc->mutex);
        
        if (send)
            _web_client_connection_send_next_request(wc);
        
    }
    
}

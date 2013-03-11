//
//  webconnection.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
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
#include "webconnection.h"

web_request_p web_request_create(const char* command, const char* path, const char* protocol);
void web_request_destroy(web_request_p wr);
void web_request_set_content(web_request_p wr, void* buffer, size_t size);

uint16_t web_response_get_status(web_response_p wr);
const char* web_response_get_status_message(web_response_p wr);
bool web_response_get_keep_alive(web_response_p wr);
size_t web_response_get_content(web_response_p wr, void* content, size_t size);

size_t web_headers_parse(web_headers_p wh, void* buffer, size_t size);

void web_server_connection_closed(web_server_p ws, web_connection_p wc);

struct web_connection_t {
    thread_p thread;
    mutex_p mutex;
    bool is_connected;
    socket_p socket;
    web_server_p server;
    web_connection_request_callback request_callback;
    void* request_callback_ctx;
    web_connection_closed_callback closed_callback;
    void* closed_callback_ctx;
};

struct web_connection_t* web_connection_create(socket_p socket, web_server_p server) {
    
    struct web_connection_t* wc = (struct web_connection_t*)malloc(sizeof(struct web_connection_t));
    bzero(wc, sizeof(struct web_connection_t));
    
    wc->socket = socket;
    wc->server = server;
    
    wc->is_connected = false;
    
    wc->request_callback = NULL;
    wc->closed_callback = NULL;
    wc->closed_callback_ctx = wc->request_callback_ctx = NULL;
    
    wc->mutex = mutex_create();
    wc->thread = NULL;
    
    return wc;
    
}

void web_connection_destroy(struct web_connection_t* wc) {
    
    mutex_lock(wc->mutex);
    if (wc->is_connected) {
        web_connection_close(wc);
        web_connection_wait_close(wc);
    }
    mutex_unlock(wc->mutex);
    
    socket_destroy(wc->socket);
    
    if (wc->thread != NULL)
        thread_destroy(wc->thread);
    
    mutex_destroy(wc->mutex);
    
    free(wc);
    
}

void web_connection_set_request_callback(struct web_connection_t* wc, web_connection_request_callback request_callback, void* ctx) {
    
    assert(request_callback);
    
    wc->request_callback = request_callback;
    wc->request_callback_ctx = ctx;
    
}

void web_connection_set_closed_callback(struct web_connection_t* wc, web_connection_closed_callback closed_callback, void* ctx) {
    
    assert(closed_callback);
    
    wc->closed_callback = closed_callback;
    wc->closed_callback_ctx = ctx;
    
}

bool web_connection_is_connected(struct web_connection_t* wc) {
    
    mutex_lock(wc->mutex);
    bool ret = wc->is_connected;
    mutex_unlock(wc->mutex);
    
    return ret;
    
}

void _web_connection_recieve_loop(void* ctx) {
    
    thread_set_name("Client connection socket");
    
    struct web_connection_t* wc = (struct web_connection_t*)ctx;
    
    size_t buffersize = 0;
    char* buffer = NULL;
    size_t readpos = 0;
    
    for (;;) {
        
        const char* content_start;
        int64_t read = 0;
        
        do {
            
            if (READ_SIZE + readpos > buffersize) {
                buffersize += READ_SIZE;
                buffer = (char*)realloc(buffer, buffersize);
            }
            
            read = socket_receive(wc->socket, &buffer[readpos], READ_SIZE);
            
            if (read <= 0)
                break;
            else
                readpos += read;
            
        } while ((content_start = web_tools_get_content_start(buffer, readpos)) == NULL);
        
        if (read <= 0)
            break;
        
        char* header_start = buffer;
        if (content_start != NULL) { // Find request end
            
            size_t header_length = web_tools_convert_new_lines(buffer, content_start - header_start);
            
            if (header_length == 2)
                break;
            
            log_data(LOG_INFO, (char*)buffer, header_length - 1);
            
            char* cmd = (char*) header_start;
            char* path = NULL;
            char* protocol = NULL;
            while (header_start[0] != '\n') {
                if (header_start[0] == ' ') {
                    if (path == NULL)
                        path = (char*)&header_start[1];
                    else if (protocol == NULL)
                        protocol = (char*)&header_start[1];
                    header_start[0] = '\0';
                }
                header_start++;
                header_length--;
            }
            
            if (!path || !protocol)
                break;
            
            header_start[0] = '\0';
            
            header_start++;
            header_length--;
            
            struct web_request_t* request = web_request_create(cmd, path, protocol);
            web_headers_p request_headers = web_request_get_headers(request);
            struct web_response_t* response = web_request_get_response(request);
            web_headers_p response_headers = web_response_get_headers(response);
            
            web_headers_parse(request_headers, header_start, content_start - header_start);
            
            const char* s_content_length;
            unsigned char* content = NULL;
            size_t content_length = 0;
            if ((s_content_length = web_headers_value(request_headers, "Content-Length")) != NULL && (content_length = atoi(s_content_length)) > 0) {
                
                content = (unsigned char*) malloc(content_length);
                size_t content_read_ros = readpos - (content_start - buffer);
                memcpy(content, content_start, content_read_ros);
                
                while (content_read_ros < content_length) {
                    
                    int64_t content_read = socket_receive(wc->socket, &content[content_read_ros], content_length - content_read_ros);
                    
                    if (content_read < 0) {
                        log_message(LOG_ERROR, "Connection read error");
                        break;
                    }
                    
                    content_read_ros += content_read;
                    
                }
                
                log_message(LOG_INFO, "(Content of length: %d bytes)\n", content_length);
                
            }
            
            assert(wc->request_callback != NULL);
            
            if (content_length > 0 && content != NULL)
                web_request_set_content(request, content, content_length);
            
            wc->request_callback(wc, request, wc->request_callback_ctx);
            
            web_response_get_headers(web_request_get_response(request));
            if (web_response_get_status(response) == 404 && web_headers_count(response_headers) == 0)
                web_headers_set_value(response_headers, "Content-Length", "0");
            
            size_t protocol_len = strlen(protocol);
            uint32_t status_code_len = 3;
            size_t status_message_len = strlen(web_response_get_status_message(response));
            
            size_t status_line_len = protocol_len + 1 + status_code_len + 1 + status_message_len + 2;
            
            size_t headers_len = web_headers_get_content(response_headers, NULL, 0);
            
            size_t total_len = status_line_len + headers_len;
            
            char* res = (char*)malloc(total_len);
            
            snprintf(res, total_len, "%s %d %s\r\n",
                     protocol,
                     web_response_get_status(response),
                     web_response_get_status_message(response));
            
            bool close_after_send = true;
            
            const char* connection = web_headers_value(request_headers, "Connection");
            
            if ((connection != NULL && strcmp(connection, "Keep-Alive") == 0)) {
                web_headers_set_value(response_headers, "Connection", "Keep-Alive");
                close_after_send = false;
            }
            
            if (web_response_get_keep_alive(response))
                close_after_send = false;
            
            web_headers_get_content(response_headers, &res[status_line_len], total_len - status_line_len);
            
            log_data(LOG_INFO, res, total_len);
            
            socket_send(wc->socket, res, total_len);
            
            free(res);
            
            size_t response_content_length;
            if ((response_content_length = web_response_get_content(response, NULL, 0))) {
                char response_content_buffer[response_content_length];
                web_response_get_content(response, response_content_buffer, response_content_length);
                log_data(LOG_INFO, (char*) response_content_buffer, response_content_length);
                socket_send(wc->socket, response_content_buffer, response_content_length);
            }
            
            if (close_after_send)
                socket_close(wc->socket);
            
            web_request_destroy(request);
            
            if (content != NULL)
                free(content);
            
        }
        
        free(buffer);
        buffer = NULL;
        buffersize = 0;
        readpos = 0;
        
    }
    
    log_message(LOG_INFO, "Client disconnected");
    
    socket_close(wc->socket);
    
    if (buffer != NULL)
        free(buffer);
    
    mutex_lock(wc->mutex);
    wc->is_connected = false;
    mutex_unlock(wc->mutex);
    
    if (wc->closed_callback != NULL)
        wc->closed_callback(wc, wc->closed_callback_ctx);
    
    web_server_connection_closed(wc->server, wc);
    
}

void web_connection_take_off(struct web_connection_t* wc) {
    
    mutex_lock(wc->mutex);
    
    wc->is_connected = true;
    wc->thread = thread_create(_web_connection_recieve_loop, wc);
    
    mutex_unlock(wc->mutex);
    
    const char *ip = sockaddr_get_host(socket_get_remote_end_point(wc->socket));
    
    log_message(LOG_INFO, "RAOPConnection (%p) took over connection from %s:%d", wc, ip, sockaddr_get_port(socket_get_remote_end_point(wc->socket)));
    
}

void web_connection_close(struct web_connection_t* wc) {
    
    if (wc->is_connected)
        socket_close(wc->socket);
    
}

void web_connection_wait_close(struct web_connection_t* wc) {
    
    if (wc->is_connected)
        thread_join(wc->thread);
    
}

struct sockaddr* web_connection_get_local_end_point(struct web_connection_t* wc) {
    
    return socket_get_local_end_point(wc->socket);
    
}

struct sockaddr* web_connection_get_remote_end_point(struct web_connection_t* wc) {
    
    return socket_get_remote_end_point(wc->socket);
    
}

//
//  webconnection.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "log.h"
#include "socket.h"
#include "sockaddr.h"
#include "webtools.h"
#include "webheaders.h"
#include "webrequest.h"
#include "webconnection.h"

struct web_connection_t {
    thread_p thread;
    pthread_mutex_t mutex;
    bool is_connected;
    socket_p socket;
    web_server_p server;
    web_connection_request_callback request_callback;
    void* request_callback_ctx;
    web_connection_closed_callback closed_callback;
    void* closed_callback_ctx;
};

void web_request_set_content(web_request_p wr, void* buffer, uint32_t size);
void web_server_connection_closed(web_server_p ws, web_connection_p wc);
uint16_t web_response_get_status(web_response_p wr);
const char* web_response_get_status_message(web_response_p wr);
bool web_response_get_keep_alive(web_response_p wr);
uint32_t web_response_get_content(web_response_p wr, void* content, uint32_t size);

struct web_connection_t* web_connection_create(socket_p socket, web_server_p server) {
    
    struct web_connection_t* wc = (struct web_connection_t*)malloc(sizeof(struct web_connection_t));
    bzero(wc, sizeof(struct web_connection_t));
    
    wc->socket = socket;
    wc->server = server;
    
    wc->is_connected = false;
    
    wc->request_callback = NULL;
    wc->closed_callback = NULL;
    wc->closed_callback_ctx = wc->request_callback_ctx = NULL;
    
    pthread_mutex_init(&wc->mutex, NULL);
    wc->thread = NULL;
    
    return wc;
    
}

void web_connection_destroy(struct web_connection_t* wc) {
    
    pthread_mutex_lock(&wc->mutex);
    if (wc->is_connected) {
        web_connection_close(wc);
        web_connection_wait_close(wc);
    }
    pthread_mutex_unlock(&wc->mutex);
    
    socket_destroy(wc->socket);
    
    if (wc->thread != NULL)
        thread_destroy(wc->thread);
    
    pthread_mutex_destroy(&wc->mutex);
    
    free(wc);
    
}

void web_connection_set_request_callback(struct web_connection_t* wc, web_connection_request_callback request_callback, void* ctx) {
    
    assert(request_callback);
    
    wc->request_callback = request_callback;
    wc->request_callback_ctx = ctx;
    
}

void web_connection_set_close_callback(struct web_connection_t* wc, web_connection_closed_callback closed_callback, void* ctx) {
    
    assert(closed_callback);
    
    wc->closed_callback = closed_callback;
    wc->closed_callback_ctx = ctx;
    
}

bool web_connection_is_connected(struct web_connection_t* wc) {
    
    pthread_mutex_lock(&wc->mutex);
    bool ret = wc->is_connected;
    pthread_mutex_unlock(&wc->mutex);
    
    return ret;
    
}

void _web_connection_recieve_loop(void* ctx) {
    
    pthread_setname_np("Client Connection Socket");
    
    struct web_connection_t* wc = (struct web_connection_t*)ctx;
    
    uint32_t buffersize = 0;
    unsigned char* buffer = NULL;
    uint32_t readpos = 0;
    
    for (;;) {
        
        unsigned char* contentStart;
        int32_t read = 0;
        
        do {
            
            if (READ_SIZE + readpos > buffersize) {
                buffersize += READ_SIZE;
                buffer = (unsigned char*)realloc(buffer, buffersize);
            }
            
            read = socket_receive(wc->socket, &buffer[readpos], READ_SIZE);
            
            if (read <= 0)
                break;
            else
                readpos += read;
            
        } while ((contentStart = web_tools_get_content_start(buffer, readpos)) == NULL);
        
        if (read <= 0)
            break;
        
        unsigned char* headerStart = buffer;
        if (contentStart != NULL) { // Find request end
            
            uint32_t headerLength = web_tools_convert_new_lines(buffer, contentStart - headerStart);
            
            if (headerLength == 2)
                break;
            
            log_data(LOG_INFO, (char*)buffer, headerLength - 1);
            
            char* cmd = (char*) headerStart;
            char* path = NULL;
            char* protocol = NULL;
            while (headerStart[0] != '\n') {
                if (headerStart[0] == ' ') {
                    if (path == NULL)
                        path = (char*)&headerStart[1];
                    else if (protocol == NULL)
                        protocol = (char*)&headerStart[1];
                    headerStart[0] = '\0';
                }
                headerStart++;
                headerLength--;
            }
            
            if (!path || !protocol)
                break;
            
            headerStart[0] = '\0';
            
            headerStart++;
            headerLength--;
            
            struct web_request_t* request = web_request_create(cmd, path, protocol);
            web_headers_p requestHeaders = web_request_get_headers(request);
            struct web_response_t* response = web_request_get_response(request);
            web_headers_p responseHeaders = web_response_get_headers(response);
            
            web_headers_parse(requestHeaders, headerStart, contentStart - headerStart);
            
            const char* contentLengthStr;
            unsigned char* content = NULL;
            uint32_t contentLength = 0;
            if ((contentLengthStr = web_headers_value(requestHeaders, "Content-Length")) != NULL && (contentLength = atoi(contentLengthStr)) > 0) {
                
                content = (unsigned char*) malloc(contentLength);
                uint32_t contentReadPos = readpos - (contentStart - buffer);
                memcpy(content, contentStart, contentReadPos);
                
                while (contentReadPos < contentLength) {
                    
                    int32_t contentRead = socket_receive(wc->socket, &content[contentReadPos], contentLength - contentReadPos);
                    
                    if (contentRead < 0) {
                        log_message(LOG_ERROR, "Connection read error");
                        break;
                    }
                    
                    contentReadPos += contentRead;
                    
                }
                
                log_message(LOG_INFO, "(Content of length: %d bytes)\n", contentLength);
                
            }
            
            assert(wc->request_callback != NULL);
            
            if (contentLength > 0 && content != NULL)
                web_request_set_content(request, content, contentLength);
            
            wc->request_callback(wc, request, wc->request_callback_ctx);
            
            web_response_get_headers(web_request_get_response(request));
            if (web_response_get_status(response) == 404 && web_headers_count(responseHeaders) == 0)
                web_headers_set_value(responseHeaders, "Content-Length", "0");
            
            uint32_t protocolLen = strlen(protocol);
            uint32_t statusCodeLen = 3;
            uint32_t statusMessageLen = strlen(web_response_get_status_message(response));
            
            uint32_t statusLineLen = protocolLen + 1 + statusCodeLen + 1 + statusMessageLen + 2;
            
            uint32_t headersLen = web_headers_get_content(responseHeaders, NULL, 0);
            
            uint32_t totalLen = statusLineLen + headersLen;
            
            char* res = (char*)malloc(totalLen);
            
            snprintf(res, totalLen, "%s %d %s\r\n",
                     protocol,
                     web_response_get_status(response),
                     web_response_get_status_message(response));
            
            bool closeAfterSend = true;
            
            const char* connection = web_headers_value(requestHeaders, "Connection");
            
            if ((connection != NULL && strcmp(connection, "Keep-Alive") == 0)) {
                web_headers_set_value(responseHeaders, "Connection", "Keep-Alive");
                closeAfterSend = false;
            }
            
            if (web_response_get_keep_alive(response))
                closeAfterSend = false;
            
            web_headers_get_content(responseHeaders, &res[statusLineLen], totalLen - statusLineLen);
            
            log_data(LOG_INFO, res, totalLen);
            
            socket_send(wc->socket, res, totalLen);
            
            free(res);
            
            uint32_t responseContentLength;
            if ((responseContentLength = web_response_get_content(response, NULL, 0))) {
                char responseContentBuffer[responseContentLength];
                web_response_get_content(response, responseContentBuffer, responseContentLength);
                log_data(LOG_INFO, (char*) responseContentBuffer, responseContentLength);
                socket_send(wc->socket, responseContentBuffer, responseContentLength);
            }
            
            if (closeAfterSend)
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
    
    pthread_mutex_lock(&wc->mutex);
    wc->is_connected = false;
    pthread_mutex_unlock(&wc->mutex);
    
    if (wc->closed_callback != NULL)
        wc->closed_callback(wc, wc->closed_callback_ctx);
    
    web_server_connection_closed(wc->server, wc);
    
    pthread_exit(0);
    
}

void web_connection_take_off(struct web_connection_t* wc) {
    
    pthread_mutex_lock(&wc->mutex);
    
    wc->is_connected = true;
    wc->thread = thread_create(_web_connection_recieve_loop, wc);
    
    pthread_mutex_unlock(&wc->mutex);
    
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

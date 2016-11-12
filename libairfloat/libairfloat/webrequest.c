//
//  webrequest.c
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
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "log.h"

#include "webtools.h"
#include "webheaders.h"
#include "webresponse.h"
#include "webrequest.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

struct web_request_t {
    char* method;
    char* path;
    char* protocol;
    void* content;
    size_t content_length;
    web_headers_p headers;
};

struct web_request_t* web_request_create() {
    
    struct web_request_t* wr = (struct web_request_t*)malloc(sizeof(struct web_request_t));
    bzero(wr, sizeof(struct web_request_t));
    
    wr->headers = web_headers_create();
        
    return wr;
    
}

void web_request_destroy(struct web_request_t* wr) {
    
    free(wr->method);
    free(wr->path);
    free(wr->protocol);
    
    if (wr->content != NULL)
        free(wr->content);
    
    web_headers_destroy(wr->headers);
    
    free(wr);
    
}

ssize_t web_request_parse(struct web_request_t* wr, const void* data, size_t data_size) {
    
    size_t ret = 0;
    
    char* buffer = malloc(data_size);
    memcpy(buffer, data, data_size);
    
    const char* content_start = web_tools_get_content_start(buffer, data_size);;
    
    if (content_start != NULL) {
        
        size_t header_length = content_start - buffer;
        char header[header_length];
        memcpy(header, buffer, content_start - buffer);
        
        log_data(LOG_COMMUNICATION, data, content_start - buffer);
        
        char* header_start = header;
        header_length = web_tools_convert_new_lines(header_start, header_length);
        
        char* cmd = header_start;
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
        
        header_start[0] = '\0';
        
        if (path == NULL || protocol == NULL) {
            free(buffer);
            return -1;
        }
        
        header_start++;
        header_length--;
                
        web_headers_p headers = web_headers_create();
        web_headers_parse(headers, header_start, header_length);
        
        size_t content_length = 0;
        const char* s_content_length = web_headers_value(headers, "Content-Length");
        if (s_content_length != NULL)
            content_length = atoi(s_content_length);
        
        size_t actual_content_length = data_size - (content_start - buffer);
        if (content_length <= actual_content_length) {
            
            wr->method = (char*)malloc(strlen(cmd) + 1);
            strcpy(wr->method, cmd);
            wr->path = (char*)malloc(strlen(path) + 1);
            strcpy(wr->path, path);
            wr->protocol = (char*)malloc(strlen(protocol) + 1);
            strcpy(wr->protocol, protocol);
            
            web_headers_destroy(wr->headers);
            wr->headers = headers;
            
            log_message(LOG_INFO, "Req: %s %s (%d bytes)", wr->method, wr->path, content_length);
            
            web_request_set_content(wr, (void*)content_start, content_length);
            
            ret = content_start + content_length - buffer;
            
        } else {
            web_headers_destroy(headers);
        }
        
    }
    
    free(buffer);
    
    return ret;
    
}

struct web_request_t* web_request_copy(struct web_request_t* wr) {
    
    struct web_request_t* request = (struct web_request_t*)malloc(sizeof(struct web_request_t));
    bzero(request, sizeof(struct web_request_t));
    
    if (wr->method != NULL) {
        request->method = malloc(strlen(wr->method) + 1);
        strcpy(request->method, wr->method);
    }
    
    if (wr->path != NULL) {
        request->path = malloc(strlen(wr->path) + 1);
        strcpy(request->path, wr->path);
    }
    
    if (wr->protocol != NULL) {
        request->protocol = malloc(strlen(wr->protocol) + 1);
        strcpy(request->protocol, wr->protocol);
    }
    
    if (wr->content != NULL && wr->content_length > 0) {
        request->content = malloc(wr->content_length);
        request->content_length = wr->content_length;
        memcpy(request->content, wr->content, wr->content_length);
    }
    
    request->headers = web_headers_copy(wr->headers);
    
    return request;
    
}

void web_request_set_method(struct web_request_t* wr, const char* method) {
    
    if (wr->method != NULL) {
        free(wr->method);
        wr->method = NULL;
    }
    
    if (method != NULL) {
        wr->method = malloc(strlen(method) + 1);
        strcpy(wr->method, method);
    }
    
}

const char* web_request_get_method(struct web_request_t* wr) {
    
    return wr->method;
    
}

void web_request_set_path(struct web_request_t* wr, const char* path) {
    
    if (wr->path != NULL) {
        free(wr->path);
        wr->path = NULL;
    }
    
    if (path != NULL) {
        wr->path = malloc(strlen(path) + 1);
        strcpy(wr->path, path);
    }
    
}

const char* web_request_get_path(struct web_request_t* wr) {
    
    return wr->path;
    
}

void web_request_set_protocol(struct web_request_t* wr, const char* protocol) {
    
    if (wr->protocol != NULL) {
        free(wr->protocol);
        wr->protocol = NULL;
    }
    
    if (protocol != NULL) {
        wr->protocol = malloc(strlen(protocol) + 1);
        strcpy(wr->protocol, protocol);
    }
    
}

const char* web_request_get_protocol(struct web_request_t* wr) {
    
    return wr->protocol;
    
}

size_t web_request_get_content(struct web_request_t* wr, void* data, size_t data_size) {
    
    if (data == NULL)
        return wr->content_length;
    
    if (wr->content != NULL) {
        
        size_t ret = MIN(wr->content_length, data_size);
        memcpy(data, wr->content, ret);
        return ret;
        
    }
    
    return 0;
    
}

void web_request_set_content(struct web_request_t* wr, const void* data, size_t data_size) {
    
    if (wr->content != NULL) {
        free(wr->content);
        wr->content = NULL;
    }
    
    wr->content_length = data_size;
    
    if (data != NULL && data_size > 0) {
        wr->content = malloc(data_size);
        memcpy(wr->content, data, data_size);
    }
    
}

web_headers_p web_request_get_headers(struct web_request_t* wr) {
    
    return wr->headers;
    
}

size_t web_request_write(struct web_request_t* wr, void* data, size_t data_size) {
    
    size_t write_pos = 0;
    
    size_t head_len = strlen(wr->method) + strlen(wr->path) + strlen(wr->protocol) + 4;
    
    if (data != NULL && write_pos + head_len <= data_size)
        sprintf(data, "%s %s %s\r\n", wr->method, wr->path, wr->protocol);
    
    write_pos += head_len;
    
    size_t headers_len = web_headers_write(wr->headers, NULL, 0);
    
    if (data != NULL && write_pos + headers_len <= data_size)
        web_headers_write(wr->headers, data + write_pos, data_size - write_pos);
    
    write_pos += headers_len;
    
    return write_pos;
    
}

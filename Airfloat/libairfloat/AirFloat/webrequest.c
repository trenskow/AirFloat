//
//  webrequest.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "webheaders.h"
#include "webresponse.h"
#include "webrequest.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

web_headers_p web_headers_create();
void web_headers_destroy(web_headers_p wh);

web_response_p web_response_create();
void web_response_destroy(web_response_p wr);

struct web_request_t {
    char* command;
    char* path;
    char* protocol;
    void* content;
    size_t content_length;
    web_headers_p headers;
    web_response_p response;
};

struct web_request_t* web_request_create(const char* command, const char* path, const char* protocol) {
    
    assert(command != NULL && path != NULL && protocol != NULL);
    
    struct web_request_t* wr = (struct web_request_t*)malloc(sizeof(struct web_request_t));
    bzero(wr, sizeof(struct web_request_t));
    
    wr->command = (char*)malloc(strlen(command) + 1);
    strcpy(wr->command, command);
    wr->path = (char*)malloc(strlen(path) + 1);
    strcpy(wr->path, path);
    wr->protocol = (char*)malloc(strlen(path) + 1);
    strcpy(wr->protocol, protocol);
    
    wr->response = web_response_create();
    wr->headers = web_headers_create();
    
    web_response_set_status(wr->response, 404, "Not Found");
    
    return wr;
    
}

void web_request_destroy(struct web_request_t* wr) {
    
    free(wr->command);
    free(wr->path);
    free(wr->protocol);
    
    if (wr->content)
        free(wr->content);
    
    web_response_destroy(wr->response);
    web_headers_destroy(wr->headers);
    
}

const char* web_request_get_command(struct web_request_t* wr) {
    
    return wr->command;
    
}

const char* web_request_get_path(struct web_request_t* wr) {
    
    return wr->path;
    
}

const char* web_request_get_protocol(struct web_request_t* wr) {
    
    return wr->protocol;
    
}

size_t web_request_get_content(struct web_request_t* wr, void* buffer, size_t size) {
    
    if (buffer == NULL)
        return wr->content_length;
    else if (wr->content != NULL) {
        
        size_t ret = MIN(wr->content_length, size);
        memcpy(buffer, wr->content, ret);
        return ret;
        
    }
    
    return 0;
    
}

void web_request_set_content(web_request_p wr, const void* buffer, size_t size) {
    
    if (wr->content != NULL) {
        free(wr->content);
        wr->content = NULL;
    }
    
    wr->content_length = size;
    
    if (buffer) {
        wr->content = malloc(size);
        memcpy(wr->content, buffer, size);
    }
    
}

web_response_p web_request_get_response(struct web_request_t* wr) {
    
    return wr->response;
    
}

web_headers_p web_request_get_headers(struct web_request_t* wr) {
    
    return wr->headers;
    
}

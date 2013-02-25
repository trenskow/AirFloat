//
//  webresponse.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "webheaders.h"
#include "webresponse.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

struct web_response_t {
    uint16_t status_code;
    char* status_message;
    
    web_headers_p headers;
    
    void* content;
    uint32_t content_length;
    
    bool keep_alive;
};

struct web_response_t* web_response_create() {
    
    struct web_response_t* wr = (struct web_response_t*)malloc(sizeof(struct web_response_t));
    bzero(wr, sizeof(struct web_response_t));
    
    wr->headers = web_headers_create();
    web_response_set_status(wr, 500, "Internal Server Error");
    
    return wr;
    
}

void web_response_destroy(struct web_response_t* wr) {
    
    if (wr->status_message != NULL)
        free(wr->status_message);
    
    if (wr->content != NULL)
        free(wr->content);
    
    web_headers_destroy(wr->headers);
    
    free(wr);
    
}

web_headers_p web_response_get_headers(struct web_response_t* wr) {
    
    return wr->headers;
    
}

void web_response_set_status(struct web_response_t* wr, uint16_t code, const char* message) {
    
    if (wr->status_message != NULL) {
        free(wr->status_message);
        wr->status_message = NULL;
    }
    
    wr->status_code = code;
    
    if (message != NULL) {
        wr->status_message = (char*)malloc(strlen(message) + 1);
        strcpy(wr->status_message, message);
    } else
        web_response_set_status(wr, 500, "Internal Server Error");
    
}

uint16_t web_response_get_status(struct web_response_t* wr) {
    
    return wr->status_code;
    
}

const char* web_response_get_status_message(struct web_response_t* wr) {
    
    return wr->status_message;
    
}

void web_response_set_content(struct web_response_t* wr, void* content, uint32_t size) {
    
    if (wr->content != NULL) {
        free(wr->content);
        wr->content = NULL;
    }
    
    if (content != NULL && size > 0) {
        
        wr->content = malloc(size);
        wr->content_length = size;
        memcpy(wr->content, content, size);
        
        web_headers_set_value(wr->headers, "Content-Length", "%d", size);
        
    } else {
        
        wr->content_length = 0;
        
        web_headers_set_value(wr->headers, "Content-Length", NULL);
        
    }
    
}

uint32_t web_response_get_content(struct web_response_t* wr, void* content, uint32_t size) {
    
    if (content)
        memcpy(content, wr->content, MIN(size, wr->content_length));
    
    return wr->content_length;
    
}

void web_response_set_keep_alive(struct web_response_t* wr, bool keep_alive) {
    
    wr->keep_alive = keep_alive;
    
}

bool web_response_get_keep_alive(struct web_response_t* wr) {
    
    return wr->keep_alive;
    
}

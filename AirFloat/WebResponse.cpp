//
//  WebResponse.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "WebResponse.h"

WebResponse::WebResponse() {
    
    _statusMessage = NULL;

    _content = NULL;
    _contentLength = 0;
    
    _keepAlive = false;
    
    _headers = new WebHeaders();
    
    setStatus(500, "Internal Server Error");
    
}

WebResponse::~WebResponse() {
    
    if (_statusMessage != NULL)
        free(_statusMessage);
    
    if (_content != NULL)
        free(_content);
    
    delete _headers;
    
}

WebHeaders* WebResponse::getHeaders() {
    
    return _headers;
    
}

void WebResponse::setStatus(uint16_t code, const char* message) {
    
    if (_statusMessage != NULL) {
        free(_statusMessage);
        _statusMessage = NULL;
    }
    
    _statusCode = code;
    
    if (message != NULL) {
        _statusMessage = (char*)malloc(strlen(message) + 1);
        strcpy(_statusMessage, message);
    } else
        setStatus(500, "Internal Server Error");
    
}

void WebResponse::setContent(void* content, uint32_t contentLength) {
    
    if (_content != NULL) {
        free(_content);
        _content = NULL;
    }
    
    _contentLength = contentLength;
    
    if (content != NULL) {
        _content = malloc(contentLength);
        memcpy(_content, content, contentLength);
    }
    
}

void WebResponse::setKeepAlive(bool keepAlive) {
    
    _keepAlive = keepAlive;
    
}

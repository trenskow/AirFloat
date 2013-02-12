//
//  WebRequest.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "WebRequest.h"
#include <assert.h>

WebRequest::WebRequest(const char* command, const char* path, const char* protocol) {
    
    _command = command;
    _path = path;
    _protocol = protocol;
    
    _content = NULL;
    _contentLength = 0;
    
    _response = new WebResponse();
    _headers = new WebHeaders();
    
    _response->setStatus(404, "Not Found");
    
}

WebRequest::~WebRequest() {
    
    if (_content != NULL) {
        free(_content);
        _content = NULL;
    }
    
    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
    
    delete _headers;
    _headers = NULL;
    
}

const char* WebRequest::getCommand() {
    
    return _command;
    
}

const char* WebRequest::getPath() {
    
    return _path;
    
}

const char* WebRequest::getProtocol() {
    
    return _protocol;
    
}

const void* WebRequest::getContent() {
    
    return _content;
    
}

const uint32_t WebRequest::getContentLength() {
    
    return _contentLength;
    
}

WebResponse* WebRequest::getResponse() {
    
    return _response;
    
}

WebHeaders* WebRequest::getHeaders() {
    
    return _headers;
    
}

void WebRequest::_setContent(const void* content, uint32_t contentLength) {
    
    if (_content != NULL) {
        free(_content);
    }
    
    _contentLength = contentLength;
    
    if (content) {
        _content = malloc(contentLength);
        memcpy(_content, content, _contentLength);
    }
    
}
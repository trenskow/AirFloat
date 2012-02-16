//
//  WebRequest.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_WebRequest_h
#define AirFloat_WebRequest_h

#include "Socket.h"
#include "WebHeaders.h"
#include "WebResponse.h"

class WebConnection;

class WebRequest {
    
    friend class WebConnection;
    
public:
    
    ~WebRequest();
    
    const char* getCommand();
    const char* getPath();
    const char* getProtocol();
    
    const void* getContent();
    const uint32_t getContentLength();
    WebResponse* getResponse();
    
    WebHeaders* getHeaders();
        
private:
    
    WebRequest(const char* command, const char* path, const char* protocol);
    
    void _setContent(const void* content, uint32_t contentLength);
    
    const char* _command;
    const char* _path;
    const char* _protocol;
    
    void* _content;
    uint32_t _contentLength;
    
    WebHeaders* _headers;
    WebResponse* _response;
    
};

#endif

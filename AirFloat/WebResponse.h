//
//  WebResponse.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_WebResponse_h
#define AirFloat_WebResponse_h

#include "WebHeaders.h"

class WebConnection;
class WebRequest;

class WebResponse {
    
    friend class WebConnection;
    friend class WebRequest;
    
public:
    
    WebResponse();
    ~WebResponse();
    
    WebHeaders* getHeaders();
    void setStatus(uint16_t code, const char* message);
    void setContent(void* content, uint32_t contentLength);
    
    void setKeepAlive(bool keepAlive);
    
private:
        
    uint16_t _statusCode;
    char* _statusMessage;
    
    WebHeaders* _headers;
    
    void* _content;
    uint32_t _contentLength;
    
    bool _keepAlive;
    
};

#endif

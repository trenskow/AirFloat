//
//  WebHeaders.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#include "WebHeaders.h"

WebHeaders::WebHeaders() {
    
    _headerCount = 0;
    _headers = NULL;
    
}

WebHeaders::~WebHeaders() {
    
    for (int h = 0 ; h < _headerCount ; h++)
        free(_headers[h].name);
    free(_headers);
    
}

const char* WebHeaders::valueForName(const char* name) {
    
    assert(name != NULL);
    
    for (int x = 0 ; x < _headerCount ; x++)
        if (0 == strcmp(_headers[x].name, name))
            return _headers[x].value;
    
    return NULL;
    
}

void WebHeaders::addValue(const char* name, const char* format, ...) {
    
    assert(name != NULL && format != NULL);

    char value[1000];
    
    va_list args;
    va_start(args, format);
    vsnprintf(value, 1000, format, args);
    va_end(args);
        
    uint32_t nameLen = strlen(name);
    uint32_t valueLen = strlen(value);
    uint32_t totalLen = nameLen + valueLen;
    
    header newHeader;
    newHeader.length = totalLen;
    newHeader.name = (char*)malloc(totalLen + 2);
    strcpy(newHeader.name, name);
    newHeader.value = &newHeader.name[nameLen + 1];
    strcpy(newHeader.value, value);
    
    _headers = (header*)realloc(_headers, sizeof(header) * (_headerCount + 1));
    _headers[_headerCount] = newHeader;
    
    _headerCount++;
        
}

uint32_t WebHeaders::getTotalLength() {
    
    uint32_t ret = 0;
    for (uint32_t i = 0 ; i < _headerCount ; i++)
        ret += _headers[i].length + 4;
    
    return ret + 5;
    
}

uint32_t WebHeaders::getContent(void* buffer, uint32_t size) {
    
    uint32_t writePos = 0;
    char* writeBuf = (char*)buffer;
    
    for (uint32_t i = 0 ; i < _headerCount ; i++) {
        
        if (size - writePos < _headers[i].length + 5)
            break;
        
        uint32_t nameLen = strlen(_headers[i].name);
        uint32_t valueLen = strlen(_headers[i].value);
        
        memcpy(&writeBuf[writePos], _headers[i].name, nameLen);
        writePos += nameLen;
        memcpy(&writeBuf[writePos], ": ", 2);
        writePos += 2;
        memcpy(&writeBuf[writePos], _headers[i].value, valueLen);
        writePos += valueLen;
        memcpy(&writeBuf[writePos], "\r\n", 3);
        writePos += 2;
        
    }
    
    if (size - writePos >= 3) {
        memcpy(&writeBuf[writePos], "\r\n", 3);
        writePos += 3;
    }
    
    return writePos;
    
}

uint32_t WebHeaders::_parseHeaders(char* buffer, uint32_t length) {
    
    assert(buffer != NULL && length > 0);
    
    char* lineStart = buffer;
    
    uint32_t i;
    for (i = 0 ; i < length ; i++)
        if (buffer[i] == '\n') {
            
            uint32_t lineLength = &buffer[i] - lineStart + 1;
            _headers = (header*)realloc(_headers, sizeof(header) * (_headerCount + 1));
            char* name = _headers[_headerCount].name = (char*)malloc(lineLength);
            memcpy(name, lineStart, lineLength);
            name[lineLength - 1] = '\0';
            
            for (uint32_t x = 0 ; x < lineLength - 1 ; x++)
                if (0 == memcmp(&name[x], ": ", 2)) {
                    _headers[_headerCount].value = &name[x + 2];
                    name[x] = '\0';
                    break;
                }
            
            _headers[_headerCount].length = lineLength - 1;
            
            _headerCount++;
            lineStart = &buffer[i + 1];
            
            if (i < length && buffer[i+1] == '\n') {
                i++;
                break;
            }
            
        }
    
    return i + 1;
    
}

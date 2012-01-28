//
//  RAOPHeader.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "RAOPHeader.h"

RAOPHeader::RAOPHeader(char* buffer, long length) {
    
    _headerCount = 0;
    _headers = NULL;
    _usedBuffer = 0;
    
    _parseBuffer(buffer, length);
    
}

RAOPHeader::~RAOPHeader() {
    
    for (int h = 0 ; h < _headerCount ; h++)
        free(_headers[h].name);
    free(_headers);
    
}

long RAOPHeader::usedBuffer() {
    
    return _usedBuffer;
    
}

const char* RAOPHeader::valueForName(const char* name) {
    
    for (int x = 0 ; x < _headerCount ; x++)
        if (0 == strcmp(_headers[x].name, name))
            return _headers[x].value;
    
    return NULL;
    
}

void RAOPHeader::_parseBuffer(char* buffer, long length) {
    
    char* lineStart = buffer;
    
    long i;
    for (i = 0 ; i < length ; i++)
        if (buffer[i] == '\n') {
            
            long lineLength = &buffer[i] - lineStart + 1;
            _headers = (header*)realloc(_headers, sizeof(header) * (_headerCount + 1));
            char* name = _headers[_headerCount].name = (char*)malloc(lineLength);
            memcpy(name, lineStart, lineLength);
            name[lineLength - 1] = '\0';
            
            for (long x = 0 ; x < lineLength - 1 ; x++)
                if (0 == memcmp(&name[x], ": ", 2)) {
                    _headers[_headerCount].value = &name[x + 2];
                    name[x] = '\0';
                    break;
                }
            
            _headerCount++;
            lineStart = &buffer[i + 1];
            
            if (i < length && buffer[i+1] == '\n') {
                i++;
                break;
            }
            
        }
    
    _usedBuffer = i + 1;
    
}

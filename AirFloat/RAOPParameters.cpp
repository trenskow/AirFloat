//
//  RAOPParameters.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "Log.h"
#include "RAOPParameters.h"

RAOPParameters::RAOPParameters(const char* buffer, long length, ParameterType type, char delimiter) {
    
    _parameters = NULL;
    _parameterCount = 0;
    
    if (type == ParameterTypeDigest)
        delimiter = ',';
    
    char parseBuf[length + 1];
    memcpy(parseBuf, buffer, length);
    if (length > 0 && parseBuf[length - 1] != delimiter) {
        parseBuf[length] = delimiter;
        length++;
    }
    
    if (type == ParameterTypeSDP)
        _parseBufferSDP(parseBuf, length, delimiter);
    else if (type == ParameterTypeTextParameters)
        _parseBufferParams(parseBuf, length);
    else if (type == ParameterTypeDigest)
        _parseBufferDigest(parseBuf, length);
    else
        log(LOG_ERROR, "RAOPParameters cannot parse data of type 0x%XX", type);
    
}

RAOPParameters::~RAOPParameters() {
    
    for (long i = 0 ; i < _parameterCount ; i++)
        free(_parameters[i].name);
    free(_parameters);
    
}

long RAOPParameters::count() {
    
    return _parameterCount;
    
}

const char* RAOPParameters::parameterAtIndex(long index) {
    
    if (index >= 0 && index < _parameterCount)
        return _parameters[index].name;

    return NULL;
    
}

const char* RAOPParameters::valueForParameter(const char* name) {
    
    for (long i = 0 ; i < _parameterCount ; i++)
        if (0 == strcmp(_parameters[i].name, name))
            return (const char*) _parameters[i].value;
    
    return NULL;
    
}

void RAOPParameters::removeParameter(const char* name) {
    
    for (long i = 0 ; i < _parameterCount ; i++)
        if (0 == strcmp(_parameters[i].name, name)) {
            for (long x = i ; x < _parameterCount - 1 ; x++)
                _parameters[x] = _parameters[x + 1];
            _parameterCount--;
            break;
        }
    
}

void RAOPParameters::_parseBufferParams(const char* buffer, long length) {
    
    char* lineStart = (char*)buffer;
    
    for (long i = 0 ; i < length ; i++)        
        if (buffer[i] == '\n') {
            
            long lineLength = &buffer[i] - lineStart;
            
            _parameters = (parameter*)realloc(_parameters, sizeof(parameter) * (_parameterCount + 1));
            char* name = _parameters[_parameterCount].name = (char*)malloc(lineLength + 1);
            memcpy(name, lineStart, lineLength);
            name[lineLength] = '\0';
            
            for (long x = 0 ; x < strlen(name) - 1 ; x++)
                if (memcmp(&name[x], ": ", 2) == 0) {
                    
                    name[x] = '\0';
                    _parameters[_parameterCount].value = &name[x+2];
                    break;                    
                }

            _parameterCount++;
            lineStart = (char*) &buffer[i+1];
            
            if (i < length && buffer[i+1] == '\n')
                break;
            
        }
    
}

void RAOPParameters::_parseBufferSDP(const char* buffer, long length, char delimiter) {
    
    char* lineStart = (char*)buffer;
    
    for (long i = 0 ; i < length ; i++) {
        
        if (buffer[i] == delimiter) {
            
            long lineLength = &buffer[i] - lineStart;
            
            _parameters = (parameter*)realloc(_parameters, sizeof(parameter) * (_parameterCount + 1));
            char* name = _parameters[_parameterCount].value = _parameters[_parameterCount].name = (char*)malloc(lineLength + 1);
            memcpy(name, lineStart, lineLength);
            name[lineLength] = '\0';
            
            for (long a = 0 ; a < strlen(name) ; a++)                
                if (name[a] == '=') {
                    bool blFound = false;
                    for (long x = a ; x < strlen(name) ; x++)
                        if (name[x] == ':') {
                            blFound = true;
                            _parameters[_parameterCount].value = &name[x+1];
                            name[x] = '\0';
                            break;                            
                        }
                    
                    if (!blFound) {
                        _parameters[_parameterCount].value = &name[a+1];
                        name[a] = '\0';
                    } else
                        name[a] = '-';

                    break;
                }
            
            _parameterCount++;
            lineStart = (char*)&buffer[i+1];
            
            if (i < length && delimiter == '\n' && buffer[i+1] == '\n')
                break;
            
        }
        
    }
    
}

void RAOPParameters::_parseBufferDigest(const char* buffer, long length) {
    
    _parseBufferSDP(buffer, length, ',');
    
    for (long i = 0 ; i < _parameterCount ; i++) {
        
        if (_parameters[i].value[0] == '"' || _parameters[i].value[0] == '\'') {
            
            _parameters[i].value++;
            _parameters[i].value[strlen(_parameters[i].value) - 1] = '\0';
            
        }
        
        size_t namelen = strlen(_parameters[i].name);
        size_t totallen = namelen + strlen(_parameters[i].value);
        
        char tmpBuf[100]; // totallen + 2];
        if (_parameters[i].name[0] == ' ') {
            totallen--; namelen--;
            memcpy(tmpBuf, &_parameters[i].name[1], namelen);
        } else
            memcpy(tmpBuf, _parameters[i].name, namelen);
        
        tmpBuf[namelen] = '\0';
        
        memcpy(&tmpBuf[namelen+1], _parameters[i].value, totallen - namelen);
        tmpBuf[totallen + 1] = '\0';
        
        memcpy(_parameters[i].name, tmpBuf, totallen + 2);
        _parameters[i].value = &_parameters[i].name[namelen+1];
        
    }
    
}

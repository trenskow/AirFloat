//
//  webheaders.c
//  AirFloat
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "webheaders.h"

struct web_header_t {
    char* name;
    char* value;
};
struct web_headers_t {
    struct web_header_t* headers;
    uint32_t count;
};

struct web_headers_t* web_headers_create() {
    
    struct web_headers_t* wh = (struct web_headers_t*)malloc(sizeof(struct web_headers_t));
    bzero(wh, sizeof(struct web_headers_t));
    
    return wh;
    
}

void web_headers_destroy(struct web_headers_t* wh) {
    
    for (uint32_t i = 0 ; i < wh->count ; i++)
        free(wh->headers[i].name);
    
    free(wh);
    
}

int32_t _web_headers_index_of(struct web_headers_t* wh, const char *name) {
    
    assert(name != NULL);
    
    for (int x = 0 ; x < wh->count ; x++)
        if (0 == strcmp(wh->headers[x].name, name))
            return x;
    
    return -1;
    
}

const char* web_headers_value(struct web_headers_t* wh, const char *name) {
    
    int32_t index = _web_headers_index_of(wh, name);
    
    if (index > -1)
        return wh->headers[index].value;
    
    return NULL;
    
}

const char* web_headers_name(struct web_headers_t* wh, uint32_t index) {
    
    assert(index < wh->count);
    
    return wh->headers[index].name;
    
}

uint32_t web_headers_count(struct web_headers_t* wh) {
    
    return wh->count;
    
}

uint32_t web_headers_get_content(struct web_headers_t* wh, void* buffer, uint32_t size) {
    
    uint32_t writePos = 0;
    char* writeBuf = (char*)buffer;
    
    for (uint32_t i = 0 ; i < wh->count ; i++) {
        
        uint32_t nameLen = strlen(wh->headers[i].name);
        uint32_t valueLen = strlen(wh->headers[i].value);
        
        if (writeBuf != NULL) {
            
            if (size - writePos < nameLen + valueLen + 4)
                break;
            
            memcpy(&writeBuf[writePos], wh->headers[i].name, nameLen);
            writePos += nameLen;
            memcpy(&writeBuf[writePos], ": ", 2);
            writePos += 2;
            memcpy(&writeBuf[writePos], wh->headers[i].value, valueLen);
            writePos += valueLen;
            memcpy(&writeBuf[writePos], "\r\n", 2);
            writePos += 2;
            
        } else
            writePos += nameLen + valueLen + 4;
        
    }
    
    if (writeBuf != NULL && size - writePos >= 2)
        memcpy(&writeBuf[writePos], "\r\n", 2);
    
    writePos += 2;
    
    return writePos;
    
}

uint32_t web_headers_parse(struct web_headers_t* wh, void* buffer, uint32_t size) {
    
    assert(buffer != NULL && size > 0);
    
    char* lineStart = buffer;
    char* readBuffer = (char*)buffer;
    
    uint32_t i;
    for (i = 0 ; i < size ; i++)
        if (readBuffer[i] == '\n') {
            
            uint32_t lineLength = &readBuffer[i] - lineStart + 1;
            wh->headers = (struct web_header_t *)realloc(wh->headers, sizeof(struct web_header_t) * (wh->count + 1));
            char* name = wh->headers[wh->count].name = (char*)malloc(lineLength);
            memcpy(name, lineStart, lineLength);
            name[lineLength - 1] = '\0';
            
            for (uint32_t x = 0 ; x < lineLength - 1 ; x++)
                if (0 == memcmp(&name[x], ": ", 2)) {
                    wh->headers[wh->count].value = &name[x + 2];
                    name[x] = '\0';
                    break;
                }
            
            wh->count++;
            lineStart = &buffer[i + 1];
            
            if (i < size && readBuffer[i+1] == '\n') {
                i++;
                break;
            }
            
        }
    
    return i + 1;
    
}

void web_headers_set_value(struct web_headers_t* wh, const char* name, const char* value, ...) {
    
    assert(name);
    
    char fvalue[1024];
    
    if (value != NULL) {
        
        va_list args;
        va_start(args, value);
        vsnprintf(fvalue, 1000, value, args);
        va_end(args);
        
    }
    
    int32_t index = _web_headers_index_of(wh, name);
    
    if (index > -1) {
        
        if (value == NULL) {
            
            free(wh->headers[index].name);
            for (uint32_t i = index + 1 ; i < wh->count ; i++)
                wh->headers[i - 1] = wh->headers[i];
            return;
            
        } else {
            
            size_t len = strlen(name) + strlen(fvalue) + 2;
            wh->headers[index].name = (char*)realloc(wh->headers[index].name, len);
            strcpy(wh->headers[index].value, fvalue);
            
        }
        
    } else {
        
        struct web_header_t newHeader;
        bzero(&newHeader, sizeof(struct web_header_t));
        
        size_t nameLen = strlen(name);
        size_t valueLen = strlen(fvalue);
        
        newHeader.name = (char*)malloc(nameLen + valueLen + 2);
        newHeader.value = &newHeader.name[nameLen + 1];
        strcpy(newHeader.name, name);
        strcpy(newHeader.value, fvalue);
        
        wh->headers = (struct web_header_t*)realloc(wh->headers, sizeof(struct web_header_t) * (wh->count + 1));
        wh->headers[wh->count] = newHeader;
        
        wh->count++;
        
    }
        
}

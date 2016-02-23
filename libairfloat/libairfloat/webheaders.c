//
//  webheaders.c
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    
    free(wh->headers);
    
    free(wh);
    
}

struct web_headers_t* web_headers_copy(struct web_headers_t* wh) {
    
    struct web_headers_t* headers = (struct web_headers_t*)malloc(sizeof(struct web_headers_t));
    bzero(headers, sizeof(struct web_headers_t));
    
    headers->count = wh->count;
    headers->headers = (struct web_header_t*)malloc(sizeof(struct web_header_t) * wh->count);
    for (uint32_t i = 0 ; i < wh->count ; i++) {
        
        size_t name_len = strlen(wh->headers[i].name);
        size_t value_len = strlen(wh->headers[i].value);
        
        headers->headers[i].name = malloc(name_len + value_len + 2);
        headers->headers[i].value = headers->headers[i].name + name_len + 1;
        
        strcpy(headers->headers[i].name, wh->headers[i].name);
        strcpy(headers->headers[i].value, wh->headers[i].value);
        
    }
    
    return headers;
    
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

size_t web_headers_parse(struct web_headers_t* wh, void* buffer, size_t size) {
    
    assert(buffer != NULL && size > 0);
    
    char* line_start = buffer;
    char* read_buffer = (char*)buffer;
    
    uint32_t i;
    for (i = 0 ; i < size ; i++)
        if (read_buffer[i] == '\n') {
            
            size_t line_length = &read_buffer[i] - line_start + 1;
            wh->headers = (struct web_header_t *)realloc(wh->headers, sizeof(struct web_header_t) * (wh->count + 1));
            char* name = wh->headers[wh->count].name = (char*)malloc(line_length);
            memcpy(name, line_start, line_length);
            name[line_length - 1] = '\0';
            
            for (uint32_t x = 0 ; x < line_length - 1 ; x++)
                if (0 == memcmp(&name[x], ": ", 2)) {
                    wh->headers[wh->count].value = &name[x + 2];
                    name[x] = '\0';
                    break;
                }
            
            wh->count++;
            line_start = buffer + i + 1;
            
            if (i < size && read_buffer[i+1] == '\n') {
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
            
            wh->count--;
            
            return;
            
        } else {
            
            size_t len = strlen(name) + strlen(fvalue) + 2;
            wh->headers[index].name = (char*)realloc(wh->headers[index].name, len);
            strcpy(wh->headers[index].value, fvalue);
            
        }
        
    } else {
        
        struct web_header_t new_header;
        bzero(&new_header, sizeof(struct web_header_t));
        
        size_t name_len = strlen(name);
        size_t value_len = strlen(fvalue);
        
        new_header.name = (char*)malloc(name_len + value_len + 2);
        new_header.value = &new_header.name[name_len + 1];
        strcpy(new_header.name, name);
        strcpy(new_header.value, fvalue);
        
        wh->headers = (struct web_header_t*)realloc(wh->headers, sizeof(struct web_header_t) * (wh->count + 1));
        wh->headers[wh->count] = new_header;
        
        wh->count++;
        
    }
        
}

size_t web_headers_write(struct web_headers_t* wh, void* data, size_t data_size) {
    
    size_t write_pos = 0;
    
    for (uint32_t i = 0 ; i < wh->count ; i++) {
        
        size_t name_len = strlen(wh->headers[i].name);
        size_t value_len = strlen(wh->headers[i].value);
        
        if (data != NULL) {
            
            if (write_pos + name_len + value_len + 4 > data_size)
                break;
            
            memcpy(data + write_pos, wh->headers[i].name, name_len);
            write_pos += name_len;
            memcpy(data + write_pos, ": ", 2);
            write_pos += 2;
            memcpy(data + write_pos, wh->headers[i].value, value_len);
            write_pos += value_len;
            memcpy(data + write_pos, "\r\n", 2);
            write_pos += 2;
            
        } else
            write_pos += name_len + value_len + 4;
        
    }
    
    if (data != NULL && write_pos + 2 <= data_size)
        memcpy(data + write_pos, "\r\n", 2);
    
    write_pos += 2;
    
    return write_pos;
    
}

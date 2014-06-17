//
//  parameters.c
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
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#include "log.h"

#include "parameters.h"

struct parameter_t {
    char* key;
    char* value;
};

struct parameters_t {
    struct parameter_t* parameters;
    uint32_t parameters_count;
};

void _parameters_parse(struct parameters_t* p, const void* data, size_t data_size, const char* delimiter, const char* key_value_seperator) {
    
    size_t delimiter_length = strlen(delimiter);
    size_t key_value_seperator_length = strlen(key_value_seperator);
    
    const char* buffer = (const char*)data;
    const char* line_start = buffer;
    
    for (size_t i = 0 ; i <= data_size ; i++) {
        
        size_t line_length = 0;
        if ((i + delimiter_length <= data_size && memcmp(&buffer[i], delimiter, delimiter_length) == 0) || i == data_size) {
            
            line_length = buffer - line_start + i;
            
            if (line_length > 0) {
                
                struct parameter_t new_parameter = { NULL, NULL };
                
                new_parameter.key = (char*)malloc(line_length + 1);
                memcpy(new_parameter.key, line_start, line_length);
                new_parameter.key[line_length] = '\0';
                new_parameter.value = strstr(new_parameter.key, key_value_seperator);
                
                if (new_parameter.value != NULL) {
                    new_parameter.value[0] = '\0';
                    new_parameter.value += key_value_seperator_length;
                    if (new_parameter.value[0] == ' ')
                        new_parameter.value++;
                    if (new_parameter.value[0] == '\"') {
                        new_parameter.value++;
                        new_parameter.key[line_length - 1] = '\0';
                    }
                }
                
                p->parameters = (struct parameter_t*)realloc(p->parameters, sizeof(struct parameter_t) * (p->parameters_count + 1));
                p->parameters[p->parameters_count] = new_parameter;
                p->parameters_count++;
                
                line_start += line_length + delimiter_length;
                
                if (line_start[0] == ' ')
                    line_start++;
                
            }
            
        }
        
    }
        
}

void _parameters_parse_http_authentication(struct parameters_t* p, const void* buffer, size_t size) {
    
    _parameters_parse(p, buffer, size, ",", "=");
    
    for (uint32_t i = 0 ; i < p->parameters_count ; i++) {
                
        if (p->parameters[i].value[0] == '"' || p->parameters[i].value[0] == '\'') {
            
            p->parameters[i].value++;
            p->parameters[i].value[strlen(p->parameters[i].value) - 1] = '\0';
            
        }
        
        size_t key_len = strlen(p->parameters[i].key);
        size_t total_len = key_len + strlen(p->parameters[i].value);
        
        char tmp_buf[total_len + 2];
        if (p->parameters[i].key[0] == ' ') {
            total_len--; key_len--;
            memcpy(tmp_buf, &p->parameters[i].key[1], key_len);
        } else
            memcpy(tmp_buf, p->parameters[i].key, key_len);
        
        tmp_buf[key_len] = '\0';
        
        memcpy(&tmp_buf[key_len+1], p->parameters[i].value, total_len - key_len);
        tmp_buf[total_len + 1] = '\0';
        
        memcpy(p->parameters[i].key, tmp_buf, total_len + 2);
        p->parameters[i].value = &p->parameters[i].key[key_len+1];
        
    }
    
}

size_t _parameters_write_http_header(struct parameters_t* p, void* buffer, size_t buffer_size) {
    
    size_t write_pos = 0;
    bzero(buffer, buffer_size);
    char* c_buffer = (char*)buffer;
    
    for (uint32_t i = 0 ; i < p->parameters_count ; i++) {
        struct parameter_t* c_param = &p->parameters[i];
        size_t key_len = strlen(c_param->key);
        if (c_param->value != NULL) {
            size_t value_len = strlen(c_param->value);
            if (buffer != NULL && write_pos + key_len + value_len + 2 <= buffer_size)
                sprintf(&c_buffer[write_pos], "%s=%s", c_param->key, c_param->value);
            write_pos += key_len + value_len + 1;
        } else {
            if (buffer != NULL && write_pos + key_len + 1 <= buffer_size)
                sprintf(&c_buffer[write_pos], "%s", c_param->key);
            write_pos += key_len;
        }
        if (i < p->parameters_count - 1) {
            if (buffer != NULL && write_pos + 2 <= buffer_size)
                strcpy(&c_buffer[write_pos], ";");
            write_pos++;
        }
    }
    
    if (buffer != NULL)
        c_buffer[write_pos] = '\0';
    
    return write_pos + 1;
    
}

struct parameters_t* parameters_create(const void* buffer, size_t size, enum parameters_type type) {
    
    struct parameters_t* p = (struct parameters_t*)malloc(sizeof(struct parameters_t));
    bzero(p, sizeof(struct parameters_t));
    memset(p, 0, sizeof(struct parameters_t));
    
    switch (type) {
        case parameters_type_text:
            _parameters_parse(p, buffer, size, "\n", ":");
            break;
        case parameters_type_sdp: {
            
            _parameters_parse(p, buffer, size, "\n", "=");
            
            for (uint32_t i = 0 ; i < p->parameters_count ; i++) {
                struct parameter_t* param = &p->parameters[i];
                if (strcmp(param->key, "a") == 0) {
                    char* colon = strchr(param->value, ':');
                    if (colon != NULL) {
                        param->key[1] = '-';
                        colon[0] = '\0';
                        param->value = colon + 1;
                    }
                }
            }
            
        } break;
        case parameters_type_http_header:
            _parameters_parse(p, buffer, size, ";", "=");
            break;
        case parameters_type_http_authentication:
            _parameters_parse_http_authentication(p, buffer, size);
            break;
    }
    
    return p;
    
}

void parameters_destroy(struct parameters_t* p) {
    
    for (uint32_t i = 0 ; i < p->parameters_count ; i++)
        free(p->parameters[i].key);
    
    free(p->parameters);
    
    free(p);
    
}

uint32_t parameters_get_count(struct parameters_t* p) {
    
    return p->parameters_count;
    
}

const char* parameters_key_at_index(struct parameters_t* p, uint32_t index) {
    
    assert(index < p->parameters_count);
    
    return p->parameters[index].key;
    
}

const char* parameters_value_for_key(struct parameters_t* p, const char* key) {
    
    for (uint32_t i = 0 ; i < p->parameters_count ; i++)
        if (0 == strcmp(p->parameters[i].key, key))
            return p->parameters[i].value;
    
    return NULL;
    
}

void parameters_set_value(struct parameters_t* p, const char* key, const char* value, ...) {
    
    assert(key != NULL && value != NULL);
    
    char new_value[100];
    bzero(new_value, 100);
    
    va_list args;
    va_start(args, value);
    
    vsnprintf(new_value, 100, value, args);
    
    va_end(args);
        
    size_t key_len = strlen(key);
    size_t value_len = strlen(new_value);
    
    for (uint32_t i = 0 ; i < p->parameters_count ; i++)
        if (0 == strcmp(p->parameters[i].key, key)) {
            struct parameter_t* c_param = &p->parameters[i];
            c_param->key = (char*)realloc(c_param->key, key_len + value_len + 2);
            c_param->value = &c_param->key[key_len + 1];
            strcpy(c_param->value, new_value);
            return;
        }
    
    p->parameters = (struct parameter_t*)realloc(p->parameters, sizeof(struct parameter_t) * (p->parameters_count + 1));
    struct parameter_t* parameter = &p->parameters[p->parameters_count];
    p->parameters_count++;
    
    parameter->key = (char*)malloc(key_len + value_len + 2);
    parameter->value = &parameter->key[key_len + 1];
    strcpy(parameter->key, key);
    strcpy(parameter->value, new_value);
    
}

void parameters_remove_key(struct parameters_t* p, const char* key) {
    
    for (uint32_t i = 0 ; i < p->parameters_count ; i++)
        if (0 == strcmp(p->parameters[i].key, key)) {
            for (uint32_t x = i ; x < p->parameters_count - 1 ; x++)
                p->parameters[x] = p->parameters[x + 1];
            p->parameters_count--;
            break;
        }
    
}

size_t parameters_write(struct parameters_t* p, void* buffer, size_t buffer_size, enum parameters_type type) {
    
    switch (type) {
        case parameters_type_http_header:
            return _parameters_write_http_header(p, buffer, buffer_size);
            break;
        default:
            assert("Write out of type is not implemented");
    }
    
    return 0;
    
}

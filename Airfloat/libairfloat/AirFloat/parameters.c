//
//  parameters.c
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
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

void _parameters_parse_text(struct parameters_t* p, const void* buffer, size_t size) {
    
    const char* start = (char*)buffer;
    const char* b = start;
    
    for (size_t i = 0 ; i < size ; i++)
        if (b[i] == '\n') {
            
            size_t length = &b[i] - start;
            
            p->parameters = (struct parameter_t*)realloc(p->parameters, sizeof(struct parameter_t) * (p->parameters_count + 1));
            char* key = p->parameters[p->parameters_count].key = (char*)malloc(length + 1);
            memcpy(key, start, length);
            key[length] = '\0';
            
            for (size_t x = 0 ; x < strlen(key) - 1 ; x++)
                if (memcmp(&key[x], ": ", 2) == 0) {
                    
                    key[x] = '\0';
                    p->parameters[p->parameters_count].value = &key[x+2];
                    
                    break;
                }
            
            p->parameters_count++;
            start = &b[i+1];
            
            if (i < size && b[i+1] == '\n')
                break;
            
        }
    
}

void _parameters_parse_simple(struct parameters_t* p, const void* buffer, size_t size, unsigned char delimiter) {
    
    const char* line_start = (char*)buffer;
    const char* b = line_start;
    
    for (size_t i = 0 ; i < size ; i++) {
        
        if (b[i] == delimiter || b[i] == '\0') {
            
            size_t line_length = &b[i] - line_start;
            
            if (line_length > 0 && line_start[0] == ' ')
                line_start++;
            
            p->parameters = (struct parameter_t*)realloc(p->parameters, sizeof(struct parameter_t) * (p->parameters_count + 1));
            char* key = p->parameters[p->parameters_count].value = p->parameters[p->parameters_count].key = (char*)malloc(line_length + 1);
            memcpy(key, line_start, line_length);
            key[line_length] = '\0';
            
            for (size_t a = 0 ; a < strlen(key) ; a++)
                if (key[a] == '=') {
                    bool blFound = false;
                    for (long x = a ; x < strlen(key) ; x++)
                        if (key[x] == ':') {
                            blFound = true;
                            p->parameters[p->parameters_count].value = &key[x+1];
                            key[x] = '\0';
                            break;
                        } else if (key[x] == ' ')
                            break;
                    
                    if (!blFound) {
                        p->parameters[p->parameters_count].value = &key[a+1];
                        key[a] = '\0';
                    } else
                        key[a] = '-';
                    
                    break;
                }
            
            p->parameters_count++;
            line_start = &b[i+1];
            
            if (i < size && delimiter == '\n' && b[i+1] == '\n')
                break;
            
        }
        
    }

}

void _parameters_parse_http_authentication(struct parameters_t* p, const void* buffer, size_t size) {
    
    _parameters_parse_simple(p, buffer, size, ',');
    
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
        if (c_param->key != c_param->value) {
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
            _parameters_parse_text(p, buffer, size);
            break;
        case parameters_type_sdp:
            _parameters_parse_simple(p, buffer, size, '\n');
            break;
        case parameters_type_http_header:
            _parameters_parse_simple(p, buffer, size, ';');
            break;
        case parameters_type_http_authentication:
            _parameters_parse_http_authentication(p, buffer, size);
            break;
    }
    
    log_message(LOG_INFO, "----");
    for (uint32_t i = 0; i < p->parameters_count; i++)
        log_message(LOG_INFO, "%s => %s", p->parameters[i].key, p->parameters[i].value);
    log_message(LOG_INFO, "----");
    
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

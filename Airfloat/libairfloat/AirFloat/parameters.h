//
//  parameters.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef _parameters_h
#define _parameters_h

#include <stdint.h>

typedef struct parameters_t *parameters_p;

enum parameters_type {
    parameters_type_text = 0,
    parameters_type_sdp,
    parameters_type_http_header,
    parameters_type_http_authentication
};

parameters_p parameters_create(const void* buffer, size_t size, enum parameters_type type);
void parameters_destroy(parameters_p p);
uint32_t parameters_get_count(parameters_p p);
const char* parameters_key_at_index(parameters_p p, uint32_t index);
const char* parameters_value_for_key(parameters_p p, const char* key);
void parameters_set_value(parameters_p p, const char* key, const char* value, ...);
void parameters_remove_key(parameters_p p, const char* key);
size_t parameters_write(parameters_p p, void* buffer, size_t buffer_size, enum parameters_type type);

#endif

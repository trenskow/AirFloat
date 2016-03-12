//
//  parameters.h
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

//
//  dmap.h
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

#ifndef _dmap_h
#define _dmap_h

#include <stdint.h>

#define DMAP_INDEX_NOT_FOUND UINT32_MAX

typedef enum {
    
    dmap_type_unknown = 0,
    dmap_type_char = 1,
    dmap_type_short = 3,
    dmap_type_long = 5,
    dmap_type_longlong = 7,
    dmap_type_string = 9,
    dmap_type_date = 10,
    dmap_type_version = 11,
    dmap_type_container = 12
    
} dmap_type;

typedef struct {
    
    uint16_t major;
    uint8_t minor;
    uint8_t patch;
    
} dmap_version;

typedef struct dmap_t *dmap_p;

dmap_p dmap_create();
void dmap_destroy(dmap_p d);
void dmap_parse(dmap_p d, const void* data, size_t data_size);
dmap_p dmap_copy(dmap_p d);

dmap_type dmap_type_for_tag(uint32_t tag);
uint32_t dmap_tag_for_identifier(const char* identifier);
const char* dmap_identifier_for_tag(uint32_t tag);

uint32_t dmap_get_count(dmap_p d);
uint32_t dmap_get_tag_at_index(dmap_p d, uint32_t index);
uint32_t dmap_get_index_of_tag(dmap_p d, uint32_t tag);

size_t dmap_get_length(dmap_p d);
size_t dmap_get_content(dmap_p d, void* content, size_t size);

size_t dmap_get_size_of_atom_at_index(dmap_p d, uint32_t index);
size_t dmap_get_size_of_atom_tag(dmap_p d, uint32_t tag);
size_t dmap_get_size_of_atom_identifer(dmap_p d, const char* identifier);

char dmap_char_at_index(dmap_p d, uint32_t index);
char dmap_char_for_atom_tag(dmap_p d, uint32_t tag);
char dmap_char_for_atom_identifer(dmap_p d, const char* identifier);
int16_t dmap_short_at_index(dmap_p d, uint32_t index);
int16_t dmap_short_for_atom_tag(dmap_p d, uint32_t tag);
int16_t dmap_short_for_atom_identifer(dmap_p d, const char* identifier);
int32_t dmap_long_at_index(dmap_p d, uint32_t index);
int32_t dmap_long_for_atom_tag(dmap_p d, uint32_t tag);
int32_t dmap_long_for_atom_identifer(dmap_p d, const char* identifier);
int64_t dmap_longlong_at_index(dmap_p d, uint32_t index);
int64_t dmap_longlong_for_atom_tag(dmap_p d, uint32_t tag);
int64_t dmap_longlong_for_atom_identifer(dmap_p d, const char* identifier);
const char* dmap_string_at_index(dmap_p d, uint32_t index);
const char* dmap_string_for_atom_tag(dmap_p d, uint32_t tag);
const char* dmap_string_for_atom_identifer(dmap_p d, const char* identifier);
uint32_t dmap_date_at_index(dmap_p d, uint32_t index);
uint32_t dmap_date_for_atom_tag(dmap_p d, uint32_t tag);
uint32_t dmap_date_for_atom_identifer(dmap_p d, const char* identifier);
dmap_version dmap_version_at_index(dmap_p d, uint32_t index);
dmap_version dmap_version_for_atom_tag(dmap_p d, uint32_t tag);
dmap_version dmap_version_for_atom_identifer(dmap_p d, const char* identifier);
dmap_p dmap_container_at_index(dmap_p d, uint32_t index);
dmap_p dmap_container_for_atom_tag(dmap_p d, uint32_t tag);
dmap_p dmap_container_for_atom_identifer(dmap_p d, const char* identifier);
const void* dmap_bytes_at_index(dmap_p d, uint32_t index);
const void* dmap_bytes_for_atom_tag(dmap_p d, uint32_t tag);
const void* dmap_bytes_for_atom_identifer(dmap_p d, const char* identifier);

void dmap_add_char(dmap_p d, int8_t chr, uint32_t tag);
void dmap_add_short(dmap_p d, int16_t shrt, uint32_t tag);
void dmap_add_long(dmap_p d, int32_t lng, uint32_t tag);
void dmap_add_longlong(dmap_p d, int64_t lnglng, uint32_t tag);
void dmap_add_string(dmap_p d, const char* string, uint32_t tag);
void dmap_add_date(dmap_p d, uint32_t date, uint32_t tag);
void dmap_add_version(dmap_p d, dmap_version version, uint32_t tag);
void dmap_add_container(dmap_p d, dmap_p container, uint32_t tag);
void dmap_add_bytes(dmap_p d, const void* data, size_t size, uint32_t tag);

#endif

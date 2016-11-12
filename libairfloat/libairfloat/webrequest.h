//
//  webrequest.h
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

#ifndef __webrequest_h
#define __webrequest_h

#include <stdint.h>

#include "webheaders.h"

typedef struct web_request_t *web_request_p;

web_request_p web_request_create();
void web_request_destroy(web_request_p wr);
ssize_t web_request_parse(web_request_p wr, const void* data, size_t data_size);
web_request_p web_request_copy(web_request_p wr);
void web_request_set_method(web_request_p wr, const char* method);
const char* web_request_get_method(web_request_p wr);
void web_request_set_path(web_request_p wr, const char* path);
const char* web_request_get_path(web_request_p wr);
void web_request_set_protocol(web_request_p wr, const char* protocol);
const char* web_request_get_protocol(web_request_p wr);
size_t web_request_get_content(web_request_p wr, void* data, size_t data_size);
void web_request_set_content(web_request_p wr, const void* data, size_t data_size);
web_headers_p web_request_get_headers(web_request_p wr);
size_t web_request_write(web_request_p wr, void* data, size_t data_size);

#endif

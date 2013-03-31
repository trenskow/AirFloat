//
//  webresponse.h
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

#ifndef __webresponse_h
#define __webresponse_h

#include "webheaders.h"

#include <stdbool.h>

typedef struct web_response_t *web_response_p;

web_response_p web_response_create();
void web_response_destroy(web_response_p wr);
ssize_t web_response_parse(web_response_p wr, const void* data, size_t data_size);
web_headers_p web_response_get_headers(web_response_p wr);
void web_response_set_status(web_response_p wr, uint16_t code, const char* message);
uint16_t web_response_get_status(web_response_p wr);
const char* web_response_get_status_message(web_response_p wr);
void web_response_set_content(web_response_p wr, void* content, size_t size);
size_t web_response_get_content(web_response_p wr, void* content, size_t size);
size_t web_response_write(web_response_p wr, const char* protocol, void* data, size_t data_size);

#endif

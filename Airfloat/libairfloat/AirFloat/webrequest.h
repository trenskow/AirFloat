//
//  webrequest.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
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
void web_request_set_command(web_request_p wr, const char* command);
const char* web_request_get_command(web_request_p wr);
void web_request_set_path(web_request_p wr, const char* path);
const char* web_request_get_path(web_request_p wr);
void web_request_set_protocol(web_request_p wr, const char* protocol);
const char* web_request_get_protocol(web_request_p wr);
size_t web_request_get_content(web_request_p wr, void* data, size_t data_size);
void web_request_set_content(web_request_p wr, const void* data, size_t data_size);
web_headers_p web_request_get_headers(web_request_p wr);
size_t web_request_write(web_request_p wr, void* data, size_t data_size);

#endif

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
#include "webresponse.h"

typedef struct web_request_t *web_request_p;

web_request_p web_request_create(const char* command, const char* path, const char* protocol);
void web_request_destroy(web_request_p wr);
const char* web_request_get_command(web_request_p wr);
const char* web_request_get_path(web_request_p wr);
const char* web_request_get_protocol(web_request_p wr);
uint32_t web_request_get_content(web_request_p wr, void* buffer, uint32_t size);
web_response_p web_request_get_response(web_request_p wr);
web_headers_p web_request_get_headers(web_request_p wr);

#endif

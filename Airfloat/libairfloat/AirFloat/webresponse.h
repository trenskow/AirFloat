//
//  webresponse.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
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

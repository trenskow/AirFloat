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
web_headers_p web_response_get_headers(web_response_p wr);
void web_response_set_status(web_response_p wr, uint16_t code, const char* message);
void web_response_set_content(web_response_p wr, void* content, uint32_t size);
void web_response_set_keep_alive(web_response_p wr, bool keep_alive);

#endif

//
//  webheaders.h
//  AirFloat
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __webheaders_h
#define __webheaders_h

#include <stdint.h>

typedef struct web_headers_t *web_headers_p;

web_headers_p web_headers_create();
void web_headers_destroy(web_headers_p wh);
web_headers_p web_headers_copy(web_headers_p wh);
const char* web_headers_value(web_headers_p wh, const char* name);
const char* web_headers_name(web_headers_p wh, uint32_t index);
uint32_t web_headers_count(web_headers_p wh);
size_t web_headers_parse(web_headers_p wh, void* buffer, size_t size);
void web_headers_set_value(web_headers_p wh, const char* name, const char* value, ...);
size_t web_headers_write(web_headers_p wh, void* data, size_t data_size);

#endif

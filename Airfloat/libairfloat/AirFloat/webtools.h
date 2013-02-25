//
//  webtools.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __webtools_h
#define __webtools_h

#include <stdint.h>

uint32_t web_tools_convert_new_lines(unsigned char* buffer, uint32_t length);
unsigned char* web_tools_get_content_start(unsigned char *buffer, uint32_t length);

#endif

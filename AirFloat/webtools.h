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

#include "webheaders.h"

size_t web_tools_convert_new_lines(void* data, size_t data_size);
const char* web_tools_get_content_start(const void* data, size_t data_size);

#endif

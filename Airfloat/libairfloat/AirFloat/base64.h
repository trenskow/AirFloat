//
//  Base64.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __BASE64_H
#define __BASE64_H

#include <stdint.h>

size_t base64_encode(const void *data, size_t size, char **str);
size_t base64_decode(const char *str, void *data);
size_t base64_pad(const char* base64, size_t base64_size, char* out, size_t out_size);

#endif
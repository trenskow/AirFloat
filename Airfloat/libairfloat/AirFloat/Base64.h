//
//  Base64.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __BASE64_H
#define __BASE64_H

int base64_encode(const void *data, int size, char **str);
int base64_decode(const char *str, void *data);

#endif
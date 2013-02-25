//
//  webtools.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>

#include "webtools.h"

uint32_t web_tools_convert_new_lines(unsigned char* buffer, uint32_t length) {
    
    for (uint32_t i = 0; i < length; i++) {
        if (buffer[i] == '\r') {
            if (i < length - 1 && buffer[i+1] == '\n') { // if newline is \r\n then remove \n. Else make \r to \n.
                memcpy(&buffer[i], &buffer[i+1], length - (i + 1));
                i--;
                length--;
            } else
                buffer[i] = '\n';
        } if (i < length - 1 && buffer[i] == '\n' && buffer[i+1] == '\n')
            break;
    }
    
    return length;
    
}

unsigned char* web_tools_get_content_start(unsigned char* buffer, uint32_t length) {
    
    for (uint32_t i = 0; i < length; i++) {
        if (i < length - 3 && memcmp(&buffer[i], "\r\n\r\n", 4) == 0)
            return &buffer[i + 4];
        if (i < length - 1 && memcmp(&buffer[i], "\n\n", 2) == 0)
            return &buffer[i + 2];
        if (i < length - 1 && memcmp(&buffer[i], "\r\r", 2) == 0)
            return &buffer[i + 2];
    }
    
    return NULL;
    
}

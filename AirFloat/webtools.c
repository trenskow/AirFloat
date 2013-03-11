//
//  webtools.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>

#include "webtools.h"

size_t web_tools_convert_new_lines(void* buffer, size_t buffer_size) {
    
    for (size_t i = 0; i < buffer_size; i++) {
        if (((char*)buffer)[i] == '\r') {
            if (i < buffer_size - 1 && ((char*)buffer)[i+1] == '\n') { // if newline is \r\n then remove \n. Else make \r to \n.
                memcpy(&buffer[i], &buffer[i+1], buffer_size - (i + 1));
                i--;
                buffer_size--;
            } else
                ((char*)buffer)[i] = '\n';
        } if (i < buffer_size - 1 && ((char*)buffer)[i] == '\n' && ((char*)buffer)[i+1] == '\n')
            break;
    }
    
    return buffer_size;
    
}

const char* web_tools_get_content_start(void* buffer, size_t buffer_size) {
    
    for (size_t i = 0; i < buffer_size; i++) {
        if (i < buffer_size - 3 && memcmp(&buffer[i], "\r\n\r\n", 4) == 0)
            return &buffer[i + 4];
        if (i < buffer_size - 1 && memcmp(&buffer[i], "\n\n", 2) == 0)
            return &buffer[i + 2];
        if (i < buffer_size - 1 && memcmp(&buffer[i], "\r\r", 2) == 0)
            return &buffer[i + 2];
    }
    
    return NULL;
    
}

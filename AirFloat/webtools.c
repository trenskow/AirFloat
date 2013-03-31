//
//  webtools.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "webheaders.h"
#include "webtools.h"

size_t web_tools_convert_new_lines(void* data, size_t data_size) {
    
    for (size_t i = 0; i < data_size; i++) {
        if (((char*)data)[i] == '\r') {
            if (i < data_size - 1 && ((char*)data)[i+1] == '\n') { // if newline is \r\n then remove \n. Else make \r to \n.
                memcpy(data + i, data + i + 1, data_size - (i + 1));
                i--;
                data_size--;
            } else
                ((char*)data)[i] = '\n';
        } if (i < data_size - 1 && ((char*)data)[i] == '\n' && ((char*)data)[i+1] == '\n')
            break;
    }
    
    return data_size;
    
}

const char* web_tools_get_content_start(const void* data, size_t data_size) {
    
    for (size_t i = 0; i < data_size; i++) {
        if (i < data_size - 3 && memcmp(data + i, "\r\n\r\n", 4) == 0)
            return data + i + 4;
        if (i < data_size - 1 && memcmp(data + i, "\n\n", 2) == 0)
            return data + i + 2;
        if (i < data_size - 1 && memcmp(data + i, "\r\r", 2) == 0)
            return data + i + 2;
    }
    
    return NULL;
    
}

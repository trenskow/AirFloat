//
//  Web.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include "WebTools.h"

uint32_t WebTools::_convertNewLines(unsigned char* buffer, uint32_t length) {
    
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

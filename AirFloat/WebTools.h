//
//  Web.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/15/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_Web_h
#define AirFloat_Web_h

#include <stdint.h>

class WebTools {
    
protected:
    static uint32_t _convertNewLines(unsigned char* buffer, uint32_t length);
    static unsigned char* _getContentStart(unsigned char* buffer, uint32_t length);
    
};

#endif

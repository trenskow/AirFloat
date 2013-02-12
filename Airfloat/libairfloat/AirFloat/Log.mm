//
//  Log.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Foundation/Foundation.h>

#include "Log.h"

void log(int level, const char* message, ...) {
    
#if defined (LOG_SERVER)
    assert((level == LOG_INFO || level == LOG_ERROR) && message != NULL);
    
//    char msgnl[strlen(message) + 2];
//    sprintf(msgnl, "%s\n", message);
    
    va_list args;
    va_start(args, message);
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSLogv([NSString stringWithCString:message encoding:NSUTF8StringEncoding], args);
    [pool release];
    
//    vprintf(msgnl, args);
    
    va_end(args);
#endif
    
}

void log_data(int level, const char* data, uint32_t size) {
    
#if defined (LOG_SERVER)
    assert((level == LOG_INFO || level == LOG_ERROR) && data != NULL && size > 0);
    
    char msgnl[size + 3];
    memcpy(msgnl, data, size);
    memcpy(&msgnl[size], "\n", 2);
    
    printf("%s", msgnl);
#endif
    
}

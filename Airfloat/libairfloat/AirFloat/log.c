//
//  log.c
//  AirFloat
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "log.h"

void log_message(int level, const char* message, ...) {
    
#if defined (LOG_SERVER) && defined(DEBUG)
    assert((level == LOG_INFO || level == LOG_ERROR) && message != NULL);
    
    char msgnl[strlen(message) + 2];
    sprintf(msgnl, "%s\n", message);
    
    va_list args;
    va_start(args, message);
    
    vprintf(msgnl, args);
    
    va_end(args);
#endif
    
}

void log_data(int level, const char* data, uint32_t size) {
    
#if defined (LOG_SERVER) && defined(DEBUG)
    assert((level == LOG_INFO || level == LOG_ERROR) && data != NULL && size > 0);
    
    char msgnl[size + 3];
    memcpy(msgnl, data, size);
    memcpy(&msgnl[size], "\n", 2);
    
    printf("%s", msgnl);
#endif
    
}

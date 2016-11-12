//
//  log.c
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "log.h"
#include "mutex.h"

#if defined (LOG_SERVER_FILE)
mutex_p write_mutex = NULL;
#endif

void log_message(int level, const char* message, ...) {
    
#if defined(LOG_SERVER)
    assert((level >= 0 && level <= LOG_ERROR) && message != NULL);
    
#if !defined(LOG_HTTP)
    if (level == LOG_COMMUNICATION) {
        return;
    }
#endif
    
    char msgnl[strlen(message) + 2];
    sprintf(msgnl, "%s\n", message);
    
    va_list args;
    va_start(args, message);
    
    vprintf(msgnl, args);
    
    va_end(args);
#endif
    
}

void log_data(int level, const void* data, size_t data_size) {
    
#if defined (LOG_SERVER)
    assert((level >= 0 && level <= LOG_ERROR) && data != NULL && data_size > 0);
    
#if !defined(LOG_HTTP)
    if (level == LOG_COMMUNICATION) {
        return;
    }
#endif
    
    char msgnl[data_size + 3];
    memcpy(msgnl, data, data_size);
    memcpy(&msgnl[data_size], "\n", 2);
    
    printf("%s", msgnl);
#endif
    
}

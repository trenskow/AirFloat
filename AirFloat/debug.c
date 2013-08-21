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

#include "debug.h"
#include "mutex.h"

mutex_p write_mutex = NULL;

const char* _fn(const char* file) {
    
    const char* ret = file;
    
    size_t len = strlen(file);
    for (size_t i = 0 ; i < len ; i++)
        if (file[i] == '/')
            ret = &file[i+1];
    
    return ret;
    
}

void _debug_data(const char* file, uint32_t line, const void* data, size_t data_size) {
    
#if defined(LOG_SERVER)
    
    if (write_mutex == NULL)
        write_mutex = mutex_create();
    
    mutex_lock(write_mutex);
    
#if defined(LOG_SERVER_FILE)
    FILE* log_file = fopen("/var/log/com.tren.AirFloat.log", "a");
#else
    FILE* log_file = stdout;
#endif
    
    fprintf(log_file, "%25s %5d ", _fn(file), line);
    
    char data_nl[data_size + 1];
    memcpy(data_nl, data, data_size);
    data_nl[data_size] = '\0';
    
    char* t = strtok(data_nl, "\n");
    if (t != NULL)
        fprintf(log_file, "%s\n", t);
    
    while (1) {
        t = strtok(NULL, "\n");
        if (t != NULL)
            fprintf(log_file, "%30s  %s", " ", t);
        else
            break;
    }
    
#if defined(LOG_SERVER_FILE)
    fclose(log_file);
#endif
    
    mutex_unlock(write_mutex);
    
#endif
    
}

void _debug(const char* file, uint32_t line, uint8_t level, const char* message, ...) {
    
#if defined(LOG_SERVER) || defined(LOG_SERVER_FILE)
    
    assert((level == LOG_INFO || level == LOG_ERROR) && message != NULL);
        
    size_t msg_len = strlen(message);
    char msg_nl[msg_len + 9];
    if (level == LOG_INFO)
        snprintf(msg_nl, msg_len + 9, "%s\n", message);
    else
        snprintf(msg_nl, msg_len + 9, "ERROR: %s\n", message);
    
    va_list args;
    va_start(args, message);
    
    char data[1024];
    vsnprintf(data, 1024, msg_nl, args);
    
    _debug_data(file, line, data, strlen(data));
    
    va_end(args);
    
#endif
    
}

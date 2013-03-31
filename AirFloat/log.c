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
#include "mutex.h"

#if defined (LOG_SERVER_FILE)
mutex_p write_mutex = NULL;
#endif

void log_message(int level, const char* message, ...) {
    
#if defined(LOG_SERVER) || defined(LOG_SERVER_FILE)
    assert((level == LOG_INFO || level == LOG_ERROR) && message != NULL);
    
    char msgnl[strlen(message) + 2];
    sprintf(msgnl, "%s\n", message);
    
    va_list args;
    va_start(args, message);
    
#if defined(LOG_SERVER)
    vprintf(msgnl, args);
#else
#if defined(LOG_SERVER_FILE)
    
    if (write_mutex == NULL)
        write_mutex = mutex_create();
    
    mutex_lock(write_mutex);
    FILE* log_file = fopen("/var/log/com.tren.AirFloat.log", "a");
    vfprintf(log_file, msgnl, args);
    fclose(log_file);
    mutex_unlock(write_mutex);
    
#endif
#endif
    
    va_end(args);
#endif
    
}

void log_data(int level, const void* data, size_t data_size) {
    
#if defined (LOG_SERVER)
    assert((level == LOG_INFO || level == LOG_ERROR) && data != NULL && data_size > 0);
    
    char msgnl[data_size + 3];
    memcpy(msgnl, data, data_size);
    memcpy(&msgnl[data_size], "\n", 2);
    
    printf("%s", msgnl);
#endif
    
}

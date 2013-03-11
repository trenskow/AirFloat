//
//  log.h
//  AirFloat
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef _log_h
#define _log_h

#include <stdint.h>

#define LOG_INFO  0
#define LOG_ERROR 1

void log_message(int level, const char* message, ...);
void log_data(int level, const void* data, size_t data_size);

#endif

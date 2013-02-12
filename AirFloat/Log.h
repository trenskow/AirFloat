//
//  Log.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __LOG_H
#define __LOG_H

#include <pthread.h>
#include <stdint.h>

#define LOG_INFO  0
#define LOG_ERROR 1

void log(int level, const char* message, ...);
void log_data(int level, const char* data, uint32_t size);

#endif
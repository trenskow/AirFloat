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

#define LOG_INFO  0
#define LOG_ERROR 1

#define enterFunc() log_enter(__func__)
#define exitFunc() log_exit(__func__)

void log(int level, const char* message, ...);
void log_enter(const char* func);
void log_exit(const char* func);
void set_log_thread(pthread_t thread);

#endif
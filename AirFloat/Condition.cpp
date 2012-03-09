//
//  Cond.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/6/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <sys/time.h>
#include <errno.h>

#include "Mutex.h"
#include "Condition.h"

Condition::Condition() {
    
    pthread_cond_init(&_cond, NULL);
    
}

Condition::~Condition() {
    
    pthread_cond_destroy(&_cond);
    
}

void Condition::wait(Mutex* mutex) {
    
    pthread_cond_wait(&_cond, &mutex->_mutex);
    
}

bool Condition::timedWait(Mutex* mutex, int milliseconds) {
    
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    int micro = milliseconds * 1000;
    while (micro >= 1000000) {
        tv.tv_sec++;
        micro -= 1000000;
    }
    
    struct timespec req = {tv.tv_sec, (tv.tv_usec + micro) * 1000};
    
    if (pthread_cond_timedwait(&_cond, &mutex->_mutex, &req) == ETIMEDOUT)
        return true;
    
    return false;
    
}

void Condition::signal() {
    
    pthread_cond_signal(&_cond);
    
}

void Condition::broadcast() {
    
    pthread_cond_broadcast(&_cond);
    
}
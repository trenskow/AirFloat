//
//  Lock.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/4/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include "Mutex.h"


Mutex::Mutex() {
    
    pthread_mutex_init(&_mutex, NULL);
    
}

Mutex::~Mutex() {
    
    pthread_mutex_destroy(&_mutex);
    
}

void Mutex::lock() {
    
    pthread_mutex_lock(&_mutex);
#ifdef DEBUG
    pthread_getname_np(pthread_self(), name, NAME_MAXLENGTH);
#endif
    
}

void Mutex::unlock() {
    
    name[0] = '\0';
    pthread_mutex_unlock(&_mutex);
    
}

Mutex::operator struct _opaque_pthread_mutex_t *() {
    
    return &_mutex;
    
}

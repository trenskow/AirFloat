//
//  Lock.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/4/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_Mutex_h
#define AirFloat_Mutex_h

#include "Condition.h"
#include <pthread.h>

#define NAME_MAXLENGTH 250

class Mutex {
    
    friend class Condition;
    
public:
    Mutex();
    ~Mutex();
    
    void lock();
    void unlock();
    
    operator pthread_mutex_t*();
    
private:
    
    char name[NAME_MAXLENGTH];
    pthread_mutex_t _mutex;
    
};

#endif

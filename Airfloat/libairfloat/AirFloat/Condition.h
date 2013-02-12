//
//  Cond.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/6/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_Condition_h
#define AirFloat_Condition_h

#include <pthread.h>

class Mutex;

class Condition {
    
public:
    Condition();
    ~Condition();
    
    void wait(Mutex* mutex);
    bool timedWait(Mutex* mutex, int milliseconds);
    
    void signal();
    void broadcast();
    
private:
    
    pthread_cond_t _cond;
    
};

#endif

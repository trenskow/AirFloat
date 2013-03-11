//
//  mutex.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/4/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef _mutex_h
#define _mutex_h

#include <stdbool.h>

typedef struct mutex_t *mutex_p;

mutex_p mutex_create();
void mutex_destroy(mutex_p m);
void mutex_lock(mutex_p m);
bool mutex_trylock(mutex_p m);
void mutex_unlock(mutex_p m);

#endif

//
//  condition.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/6/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef _condition_h
#define _condition_h

#include <stdbool.h>

#include "mutex.h"

typedef struct condition_t *condition_p;

condition_p condition_create();
void condition_destroy(condition_p c);
void condition_wait(condition_p c, mutex_p mutex);
bool condition_times_wait(condition_p c, mutex_p mutex, int milliseconds);
void condition_signal(condition_p c);
void condition_broadcast(condition_p c);

#endif

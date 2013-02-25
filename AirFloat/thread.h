//
//  thread.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/25/13.
//
//

#ifndef _thread_h
#define _thread_h

#include <stdint.h>

typedef void (*thread_start_fnc)(void* ctx);
typedef struct thread_t *thread_p;

thread_p thread_create(thread_start_fnc start_fnc, void* ctx);
void thread_destroy(thread_p t);
void thread_set_name(const char* name);
void thread_join(thread_p t);

#endif

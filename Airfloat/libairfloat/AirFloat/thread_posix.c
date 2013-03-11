//
//  thread_posix.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/25/13.
//
//

#if (__APPLE__)

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "log.h"
#include "condition.h"
#include "thread.h"

struct thread_t {
    thread_start_fnc fnc;
    void* ctx;
    pthread_t* thread;
};


void* thread_pthread_head(void* ctx) {
    
    struct thread_t* t = (struct thread_t*)ctx;
    
    t->fnc(t->ctx);
        
    pthread_exit(0);
    
}

struct thread_t* thread_create(thread_start_fnc start_fnc, void* ctx) {
    
    struct thread_t* t = (struct thread_t*)malloc(sizeof(struct thread_t));
    
    t->fnc = start_fnc;
    t->ctx = ctx;
    t->thread = (pthread_t*)malloc(sizeof(pthread_t));
    
    pthread_create(t->thread, NULL, thread_pthread_head, t);
    
    return t;
    
}

void thread_destroy(struct thread_t* t) {
    
    if (t->thread != NULL) {
        pthread_join(*t->thread, NULL);
        free(t->thread);
    }
    
    free(t);
    
}

void thread_set_name(const char* name) {
    
    pthread_setname_np(name);
    
}

void thread_join(struct thread_t* t) {
    
    if (t->thread != NULL)
        pthread_join(*t->thread, NULL);
    
}

#endif

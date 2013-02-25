//
//  struct condition_t*osix.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/6/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#if (__APPLE__)

#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

#include "condition.h"

pthread_mutex_t* mutex_pthread(struct mutex_t* m);

struct condition_t {
    pthread_cond_t cond;
};

struct condition_t* condition_create() {
    
    struct condition_t* c = (struct condition_t*)malloc(sizeof(struct condition_t));
    
    pthread_cond_init(&c->cond, NULL);
    
    return c;
    
}

void condition_destroy(struct condition_t* c) {
    
    pthread_cond_destroy(&c->cond);
    free(c);
    
}

void condition_wait(struct condition_t* c, mutex_p mutex) {
    
    pthread_cond_wait(&c->cond, mutex_pthread(mutex));
    
}

bool condition_times_wait(struct condition_t* c, mutex_p mutex, int milliseconds) {
    
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    
    int micro = milliseconds * 1000;
    while (micro >= 1000000) {
        tv.tv_sec++;
        micro -= 1000000;
    }
    
    struct timespec req = {tv.tv_sec, (tv.tv_usec + micro) * 1000};
    
    if (pthread_cond_timedwait(&c->cond, mutex_pthread(mutex), &req) == ETIMEDOUT)
        return true;
    
    return false;
    
}

void condition_signal(struct condition_t* c) {
    
    pthread_cond_signal(&c->cond);
    
}

void condition_broadcast(struct condition_t* c) {
    
    pthread_cond_broadcast(&c->cond);
    
}

#endif

//
//  mutex_posix.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/4/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#if (__APPLE__)

#include <stdlib.h>
#include <pthread.h>

#include "mutex.h"

#define MAX_NAME_LENGTH 250

struct mutex_t {
    pthread_mutex_t mutex;
    char name[MAX_NAME_LENGTH];
};

struct mutex_t* mutex_create() {
    
    struct mutex_t* m = (struct mutex_t*)malloc(sizeof(struct mutex_t));
    
    pthread_mutex_init(&m->mutex, NULL);
    
    return m;
    
}

void mutex_destroy(struct mutex_t* m) {
    
    pthread_mutex_destroy(&m->mutex);
    free(m);
    
}

bool mutex_trylock(struct mutex_t* m) {
    
    return (pthread_mutex_trylock(&m->mutex) == 0);
    
}

void mutex_lock(struct mutex_t* m) {
    
    pthread_mutex_lock(&m->mutex);
#ifdef DEBUG
    pthread_getname_np(pthread_self(), m->name, MAX_NAME_LENGTH);
#endif
    
}

void mutex_unlock(struct mutex_t* m) {
    
#ifdef DEBUG
    m->name[0] = '\0';
    pthread_mutex_unlock(&m->mutex);
#endif
    
}

pthread_mutex_t* mutex_pthread(struct mutex_t* m) {
    
    return &m->mutex;
    
}

#endif

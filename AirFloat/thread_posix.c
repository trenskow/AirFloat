//
//  thread_posix.c
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#if (__APPLE__)

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "log.h"
#include "condition.h"

#include "obj.h"

#include "thread.h"

struct thread_t {
    thread_start_fnc fnc;
    void* ctx;
    bool finir;
    pthread_mutex_t mutex;
    pthread_cond_t join_cond;
    pthread_t thread;
};


void* thread_pthread_head(void* ctx) {
    
    struct thread_t* t = (struct thread_t*)ctx;
    
    thread_retain(t);
    
    t->fnc(t->ctx);
    
    pthread_mutex_lock(&t->mutex);
    
    t->finir = true;
    pthread_cond_broadcast(&t->join_cond);
    
    pthread_mutex_unlock(&t->mutex);
    
    thread_release(t);
    
    pthread_exit(0);
    
}

struct thread_t* thread_create(thread_start_fnc start_fnc, void* ctx) {
    
    struct thread_t* t = (struct thread_t*)obj_create(sizeof(struct thread_t));
    
    t->fnc = start_fnc;
    t->ctx = ctx;
    
    pthread_mutex_init(&t->mutex, NULL);
    pthread_cond_init(&t->join_cond, NULL);
    
    pthread_create(&t->thread, NULL, thread_pthread_head, t);
    
    return t;
    
}

void _thread_destroy(void* obj) {
    
    struct thread_t* t = (struct thread_t*)obj;
    
    thread_join(t);
    
    pthread_cond_destroy(&t->join_cond);
    pthread_mutex_destroy(&t->mutex);
    
}

thread_p thread_retain(thread_p t) {
    
    return obj_retain(t);
    
}

struct thread_t* thread_release(thread_p t) {
    
    return obj_release(t, NULL);
    
}

void thread_set_name(const char* name) {
    
    pthread_setname_np(name);
    
}

void thread_join(struct thread_t* t) {
    
    pthread_mutex_lock(&t->mutex);
    if (!t->finir)
        pthread_cond_wait(&t->join_cond, &t->mutex);
    pthread_mutex_unlock(&t->mutex);
    
}

#endif

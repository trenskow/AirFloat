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

struct thread_t* thread_create_a(thread_start_fnc start_fnc, void* ctx) {
    
    struct thread_t* t = (struct thread_t*)malloc(sizeof(struct thread_t));
    
    t->fnc = start_fnc;
    t->ctx = ctx;
    t->thread = (pthread_t*)malloc(sizeof(pthread_t));
    
    pthread_create(t->thread, NULL, thread_pthread_head, t);
    
    return t;
    
}

void thread_destroy(struct thread_t* t) {
    
    thread_join(t);
    
    free(t);
    
}

void thread_set_name(const char* name) {
    
    pthread_setname_np(name);
    
}

void thread_join(struct thread_t* t) {
    
    if (t->thread != NULL) {
        pthread_join(*t->thread, NULL);
        free(t->thread);
        t->thread = NULL;
    }
    
}

#endif

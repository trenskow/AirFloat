//
//  obj_posix.c
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

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "log.h"
#include "obj.h"

struct obj_t {
    uint32_t retain_count;
    pthread_mutex_t mutex;
};

void* obj_create(size_t size) {
    
    struct obj_t* obj = malloc(sizeof(struct obj_t) + size);
    bzero(obj, sizeof(struct obj_t) + size);
    
    obj->retain_count = 1;
    pthread_mutex_init(&obj->mutex, NULL);
    
    return obj + sizeof(struct obj_t);
    
}

void* obj_retain(void* o) {
    
    struct obj_t* obj = o - sizeof(struct obj_t);
    
    pthread_mutex_lock(&obj->mutex);
    
    obj->retain_count++;
    
    pthread_mutex_unlock(&obj->mutex);
    
    return o;
    
}

void obj_release(void* o, destroy_callback destroy) {
    
    struct obj_t* obj = o - sizeof(struct obj_t);
    
    pthread_mutex_lock(&obj->mutex);
    
    if (!--obj->retain_count) {
        
        destroy(o);
        
        pthread_mutex_unlock(&obj->mutex);
        pthread_mutex_destroy(&obj->mutex);
        
        free(obj);
        
    } else
        pthread_mutex_unlock(&obj->mutex);
    
}

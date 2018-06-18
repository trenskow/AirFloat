//
//  object.c
//  libairfloat
//
//  Created by Kristian Trenskow on 18/06/2018.
//

#include <assert.h>
#include <string.h>

#include "mutex.h"

#include "object.h"

struct object_t {
    size_t retain_count;
    mutex_p mutex;
    void(*destroyer)(void*);
};

struct object_t* _object_get(void* data) {
    return ((struct object_t **)data)[0];
}

void* object_create(size_t size, void(*destroyer)(void *)) {
    
    void* data = malloc(size);
    bzero(data, size);
    
    ((struct object_t **)data)[0] = (struct object_t*)malloc(sizeof(struct object_t));
    
    struct object_t* object = _object_get(data);
    
    object->retain_count = 1;
    object->mutex = mutex_create();
    object->destroyer = destroyer;
    
    return data;
    
}

void* object_retain(void* data) {
    struct object_t* object = _object_get(data);
    mutex_lock(object->mutex);
    object->retain_count++;
    mutex_unlock(object->mutex);
    return data;
}

void object_release(void* data) {
    struct object_t* object = _object_get(data);
    mutex_lock(object->mutex);
    object->retain_count--;
    bool destroy = !object->retain_count;
    if (destroy && object->destroyer) {
        object->destroyer(data);
    }
    mutex_unlock(object->mutex);
    if (destroy) {
        mutex_destroy(object->mutex);
        free(object);
        free(data);
    }
}

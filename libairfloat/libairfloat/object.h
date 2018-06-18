//
//  object.h
//  libairfloat
//
//  Created by Kristian Trenskow on 18/06/2018.
//

#ifndef object_h
#define object_h

#include <stdlib.h>

typedef struct object_t *object_p;

void* object_create(size_t size, void(*destroyer)(void *));
void* object_retain(void* object);
void object_release(void* object);

#endif /* object_h */

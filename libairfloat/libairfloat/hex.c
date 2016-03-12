//
//  hex.c
//  libairfloat
//
//  Created by Kristian Trenskow on 13/08/15.
//
//

#include <assert.h>

#include "hex.h"

void hex_encode(const void* content, size_t content_size, void* hex, size_t hex_size) {
    
    assert(hex_size >= content_size * 2);
    
    static char hex_values[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    
    for (int i = 0 ; i < content_size ; i++) {
        ((char*)hex)[i*2] = hex_values[(((char*)content)[i] >> 4) & 0xF];
        ((char*)hex)[(i*2)+1] = hex_values[((char*)content)[i] & 0xF];
    }
    
}

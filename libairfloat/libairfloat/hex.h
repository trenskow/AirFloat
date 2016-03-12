//
//  hex.h
//  libairfloat
//
//  Created by Kristian Trenskow on 13/08/15.
//
//

#ifndef __HEX_H
#define __HEX_H

#include <stdint.h>

void hex_encode(const void* content, size_t content_size, void* hex, size_t hex_size);

#endif /* defined(__libairfloat__hex__) */

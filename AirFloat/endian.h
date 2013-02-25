//
//  endian.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef _endian_h
#define _endian_h

#if __CHAR_BIT__ != 8
#error AirFloat does not support non-8-bit systems
#endif

#include <stdint.h>

static const union { unsigned char bytes[4]; uint32_t value; } big_endian_long = { { 0, 0, 0, 1 } };

#define big_endian_order 0x00000001
#define machine_byte_order big_endian_long.value
#define machine_is_big machine_byte_order == big_endian_order

#define uint16_t_swp(s) ((s & 0x00FF) << 8 | ((s & 0xFF00) >> 8))
#define uint32_t_swp(l) ((l & 0x000000FF) << 24 | (l & 0x0000FF00) << 8 | (l & 0x00FF0000) >> 8 | (l & 0xFF000000) >> 24)
#define uint64_t_swp(l) ((l & 0x00000000000000FF) << 56 | (l & 0x000000000000FF00) << 40 | (l & 0x0000000000FF0000) << 24 | \
(l & 0x00000000FF000000) << 8  | (l & 0x000000FF00000000) >> 8  | (l & 0x0000FF0000000000) >> 24 | \
(l & 0x00FF000000000000) >> 40 | (l & 0xFF00000000000000) >> 56)

#define btms(s)  (machine_is_big ? s : uint16_t_swp(s))
#define btml(l)  (machine_is_big ? l : uint32_t_swp(l))
#define btmll(l) (machine_is_big ? l : uint64_t_swp(l))

#define mtbs(s)  (machine_is_big ? s : uint16_t_swp(s))
#define mtbl(l)  (machine_is_big ? l : uint32_t_swp(l))
#define mtbll(l) (machine_is_big ? l : uint64_t_swp(l))

#endif

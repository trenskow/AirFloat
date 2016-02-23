//
//  endian.h
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

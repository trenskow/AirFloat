//
//  alac_format.h
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

#ifndef _alac_format_h
#define _alac_format_h

#include <stdint.h>

struct alac_specific_config_t {
    uint32_t        frame_length;
    uint8_t         compatible_version;
    uint8_t         bit_depth;
    uint8_t         pb;
    uint8_t         mb;
    uint8_t         kb;
    uint8_t         num_channels;
    uint16_t        max_run;
    uint32_t        max_frame_bytes;
    uint32_t        avg_bit_rate;
    uint32_t        sample_rate;
};

struct alac_magic_cookie_t {
    
    struct {
        uint32_t atom_size;
        uint32_t channel_layout_info_id;
        uint32_t type;
    } format_atom;
    struct {
        uint32_t info_size;
        uint32_t id;
        uint32_t version_flag;
        struct alac_specific_config_t config;
    } alac_specific_info;
    struct {
        uint32_t channel_layout_info_size;
        uint32_t channel_layout_info_id;
    } terminator_atom;
    
};

struct alac_magic_cookie_t alac_format_parse(const char* rtp_fmtp);

#endif

//
//  alac_format.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/25/13.
//
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
        uint32_t atomSize;
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

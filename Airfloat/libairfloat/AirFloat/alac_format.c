//
//  alac_format.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/25/13.
//
//

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "endian.h"
#include "alac_format.h"

struct alac_magic_cookie_t alac_format_parse(const char* rtp_fmtp) {
    
    size_t fmtp_len = strlen(rtp_fmtp);
    char fmtp_s[fmtp_len + 1];
    strcpy(fmtp_s, rtp_fmtp);
    
    uint32_t fmtp[11];
    uint32_t fmtp_c = 0;
    
    char* read_pos = fmtp_s;
    
    for (uint32_t i = 0 ; true ; i++) {
        if (fmtp_s[i] == ' ') {
            fmtp_s[i] = '\0';
            fmtp[fmtp_c++] = atoi(read_pos);
            read_pos = &fmtp_s[i + 1];
        } else if (fmtp_s[i] == '\0') {
            fmtp[fmtp_c++] = atoi(read_pos);
            break;
        }
    }
    
    struct alac_magic_cookie_t cookie;
    bzero(&cookie, sizeof(struct alac_magic_cookie_t));
    
    cookie.format_atom.atomSize = mtbl(12);
    cookie.format_atom.channel_layout_info_id = mtbl('frma');
    cookie.format_atom.type = mtbl('alac');
    cookie.alac_specific_info.info_size = mtbl(36);
    cookie.alac_specific_info.id = mtbl('alac');
    cookie.alac_specific_info.version_flag = 0;
    cookie.alac_specific_info.config.frame_length = mtbl(fmtp[0]);
    cookie.alac_specific_info.config.compatible_version = fmtp[1];
    cookie.alac_specific_info.config.bit_depth = fmtp[2];
    cookie.alac_specific_info.config.pb = fmtp[3];
    cookie.alac_specific_info.config.mb = fmtp[4];
    cookie.alac_specific_info.config.kb = fmtp[5];
    cookie.alac_specific_info.config.num_channels = fmtp[6];
    cookie.alac_specific_info.config.max_run = mtbs((uint16_t)fmtp[7]);
    cookie.alac_specific_info.config.max_frame_bytes = mtbl(fmtp[8]);
    cookie.alac_specific_info.config.avg_bit_rate = mtbl(fmtp[9]);
    cookie.alac_specific_info.config.sample_rate = mtbl(fmtp[10]);
    cookie.terminator_atom.channel_layout_info_size = mtbl(8);
    cookie.terminator_atom.channel_layout_info_id = 0;
    
    return cookie;
    
}

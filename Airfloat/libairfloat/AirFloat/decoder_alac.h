//
//  decoder_alac.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/25/13.
//
//

#ifndef _decoder_alac_h
#define _decoder_alac_h

#include <stdint.h>

#include "decoder.h"

void* decoder_alac_create(const char* rtp_fmtp);
void decoder_alac_destroy(void* data);
struct decoder_output_format_t decoder_alac_get_output_format(void* data);
size_t decoder_alac_decode(void* data, void* in_audio_data, size_t in_audio_data_size, void* out_audio_data, size_t out_audio_data_size);
void decoder_alac_reset(void* data);

#endif

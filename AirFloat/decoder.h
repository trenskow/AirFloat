//
//  decoder.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/25/13.
//
//

#ifndef _decoder_h
#define _decoder_h

#include <stdint.h>

struct decoder_output_format_t {
    uint32_t frames_per_packet;
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t bit_depth;
    uint32_t frame_size;
};

typedef struct decoder_t *decoder_p;

decoder_p decoder_create(const char* type, const char* rtp_fmtp);
void decoder_destroy(decoder_p d);
struct decoder_output_format_t decoder_get_output_format(decoder_p d);
size_t decoder_decode(decoder_p d, void* in_audio_data, size_t in_audio_data_size, void* out_audio_data, size_t out_audio_data_size);
void decoder_reset(decoder_p d);

#endif

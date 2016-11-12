//
//  decoder_alac_other.c
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

#if !defined(__APPLE__)

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "endian.h"
#include "alac.h"
#include "decoder.h"
#include "alac_format.h"

#define MIN(x, y) (x < y ? x : y)

struct decoder_alac_other_t {
    struct alac_magic_cookie_t magic_cookie;
    struct decoder_output_format_t output_format;
    alac_file* alac;
};

void decoder_alac_reset(void* data);

void* decoder_alac_create(const char* rtp_fmtp) {
    
    struct decoder_alac_other_t* d = (struct decoder_alac_other_t*)malloc(sizeof(struct decoder_alac_other_t));
    bzero(d, sizeof(struct decoder_alac_other_t));
    
    d->magic_cookie = alac_format_parse(rtp_fmtp);
    
    decoder_alac_reset(d);
    
    return d;
    
}

void decoder_alac_destroy(void* data) {
    
    struct decoder_alac_other_t* d = (struct decoder_alac_other_t*)data;
    
    deallocate_buffers(d->alac);
    dispose_alac(d->alac);
    
    free(d);
    
}

struct decoder_output_format_t decoder_alac_get_output_format(void* data) {
    
    return ((struct decoder_alac_other_t*)data)->output_format;
    
}

size_t decoder_alac_decode(void* data, void* in_audio_data, size_t in_audio_data_size, void* out_audio_data, size_t out_audio_data_size) {
    
    int output_size = (int)out_audio_data_size;
    decode_frame(((struct decoder_alac_other_t*)data)->alac, (unsigned char*)in_audio_data, (unsigned char*)out_audio_data, &output_size);
    
    return output_size;
    
}

void decoder_alac_reset(void* data) {
    
    struct decoder_alac_other_t* d = (struct decoder_alac_other_t*)data;
    
    if (d->alac != NULL) {
        deallocate_buffers(d->alac);
        dispose_alac(d->alac);
        d->alac = NULL;
    }
    
    struct alac_specific_config_t* config = &d->magic_cookie.alac_specific_info.config;
    
    d->alac = create_alac(config->bit_depth, 2);
    
    d->alac->setinfo_max_samples_per_frame = d->output_format.frames_per_packet = btml(config->frame_length);;
    d->alac->setinfo_7a = config->compatible_version;
    d->alac->setinfo_sample_size = d->output_format.bit_depth = config->bit_depth;
    d->alac->setinfo_rice_historymult = config->pb;
    d->alac->setinfo_rice_initialhistory = config->mb;
    d->alac->setinfo_rice_kmodifier = config->kb;
    d->alac->setinfo_7f = d->output_format.channels = config->num_channels;
    d->alac->setinfo_80 = btms(config->max_run);
    d->alac->setinfo_82 = btml(config->max_frame_bytes);
    d->alac->setinfo_86 = btml(config->avg_bit_rate);
    d->alac->setinfo_8a_rate = d->output_format.sample_rate = btml(config->sample_rate);
    
    d->output_format.frame_size = d->output_format.channels * (d->output_format.bit_depth / 8);
    
    allocate_buffers(d->alac);
    
}

#endif

//
//  decoder.c
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "mutex.h"
#include "decoder_alac.h"

enum decoder_type {
    decoder_type_unknown = 0,
    decoder_type_alac
};

struct decoder_t {
    enum decoder_type type;
    mutex_p mutex;
    void* data;
};

struct decoder_t* decoder_create(const char* type, const char* rtp_fmtp) {
    
    assert(type);

    struct decoder_t* d = (struct decoder_t*)malloc(sizeof(struct decoder_t));
    bzero(d, sizeof(struct decoder_t));
    
    d->mutex = mutex_create();
    
    if (strcmp(type, "AppleLossless") == 0) {
        
        d->type = decoder_type_alac;
        d->data = decoder_alac_create(rtp_fmtp);
        
    }
    
    return d;
    
}

void decoder_destroy(struct decoder_t* d) {
    
    if (d->type == decoder_type_alac)
        decoder_alac_destroy(d->data);
    
    mutex_destroy(d->mutex);
    
    free(d);
    
}

struct decoder_output_format_t decoder_get_output_format(decoder_p d) {
    
    if (d->type == decoder_type_alac)
        return decoder_alac_get_output_format(d->data);
    
    return (struct decoder_output_format_t){0,0,0,0};
    
}

size_t decoder_decode(struct decoder_t* d, void* in_audio_data, size_t in_audio_data_size, void* out_audio_data, size_t out_audio_data_size) {
    
    size_t ret = 0;
    
    mutex_lock(d->mutex);
    
    if (d->type == decoder_type_alac)
        ret = decoder_alac_decode(d->data, in_audio_data, in_audio_data_size, out_audio_data, out_audio_data_size);
    
    mutex_unlock(d->mutex);
    
    return ret;
    
}

void decoder_reset(struct decoder_t* d) {
    
    mutex_lock(d->mutex);
    
    if (d->type == decoder_type_alac)
        decoder_alac_reset(d->data);
    
    mutex_unlock(d->mutex);
    
}

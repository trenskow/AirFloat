//
//  audiooutput.h
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

#ifndef audiooutput_h
#define audiooutput_h

#include <stdint.h>

#include "decoder.h"

typedef struct audio_output_t *audio_output_p;

typedef void (*audio_output_callback)(audio_output_p ao, void* buffer, size_t size, double host_time, void* ctx);

double audio_output_get_host_time();

audio_output_p audio_output_create(struct decoder_output_format_t decoder_output_format);
audio_output_p audio_output_retain(audio_output_p ao);
audio_output_p audio_output_release(audio_output_p ao);

void audio_output_set_callback(audio_output_p ao, audio_output_callback callback, void* ctx);
void audio_output_start(audio_output_p ao);
void audio_output_stop(audio_output_p ao);
void audio_output_flush(audio_output_p ao);
double audio_output_get_playback_rate(audio_output_p ao);
void audio_output_set_playback_rate(audio_output_p ao, double playback_rate);
void audio_output_set_volume(audio_output_p ao, double volume);
void audio_output_set_muted(audio_output_p ao, bool muted);

#endif

//
//  audioqueue.h
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

#ifndef AirFloat_AudioQueue_h
#define AirFloat_AudioQueue_h

#include <stdint.h>
#include <stdbool.h>

#include "audiooutput.h"
#include "decoder.h"

typedef struct audio_queue_t *audio_queue_p;

typedef void(*audio_queue_received_audio_callback)(audio_queue_p aq, void* ctx);

struct audio_queue_missing_packet_window {
    uint16_t seq_no;
    uint16_t packet_count;
};

audio_queue_p audio_queue_create(decoder_p decoder);
void audio_queue_destroy(audio_queue_p aq);
void audio_queue_set_received_audio_callback(audio_queue_p aq, audio_queue_received_audio_callback callback, void* ctx);
void audio_queue_disable_synchronization(audio_queue_p aq);
void audio_queue_synchronize(audio_queue_p aq, uint32_t current_sample_time, double current_time, uint32_t next_sample_time);
void audio_queue_set_remote_time(audio_queue_p aq, double remote_time);
uint32_t audio_queue_add_packet(audio_queue_p aq, void* encoded_buffer, size_t encoded_buffer_size, uint16_t seq_no, uint32_t sample_time);
struct audio_queue_missing_packet_window audio_queue_get_next_missing_window(audio_queue_p aq);
void audio_queue_start(audio_queue_p aq);
void audio_queue_flush(audio_queue_p aq, uint16_t last_seq_no);
bool audio_queue_wait_for_space(audio_queue_p aq);
audio_output_p audio_queue_get_output(audio_queue_p aq);

#endif

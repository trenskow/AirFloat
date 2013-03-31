//
//  AudioQueue.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
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

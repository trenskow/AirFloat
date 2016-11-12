//
//  audioqueue.c
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
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "log.h"
#include "mutex.h"
#include "condition.h"
#include "decoder.h"
#include "hardware.h"
#include "audiooutput.h"
#include "audioqueue.h"

//#define MAX_QUEUE_COUNT 500
#define MAX_QUEUE_COUNT 4096
#define CLIENT_SERVER_DIFFERENCE_BACKLOG 10
#define LOOP_FROM(x, y, d, c) for (struct audio_packet_t* x = y ; x != c ; x = x->d)
#define IS_UPPER(x) (((x % 0xFFFF) & 0x8000) != 0)
#define IS_LOWER(x) (((x % 0xFFFF) & 0xC000) == 0)
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

typedef void (*audio_output_callback)(audio_output_p ao, void* buffer, size_t size, double host_time, void* ctx);

double audio_output_get_host_time();

audio_output_p audio_output_create(struct decoder_output_format_t decoder_output_format);
void audio_output_destroy(audio_output_p ao);
void audio_output_set_callback(audio_output_p ao, audio_output_callback callback, void* ctx);
void audio_output_start(audio_output_p ao);
void audio_output_stop(audio_output_p ao);
void audio_output_flush(audio_output_p ao);
double audio_output_get_playback_rate(audio_output_p ao);
void audio_output_set_playback_rate(audio_output_p ao, double playback_rate);

enum audio_packet_state {
    audio_packet_state_complete = 0,
    audio_packet_state_missing,
    audio_packet_state_requested
};

struct audio_packet_t {
    uint16_t seq_no;
    uint32_t sample_time;
    double time;
    enum audio_packet_state state;
    struct audio_packet_t* next;
    struct audio_packet_t* previous;
    void* buffer;
    size_t buffer_size;
    mutex_p mutex;
};

struct audio_packet_t* audio_packet_create(enum audio_packet_state state) {
    
    struct audio_packet_t* ap = (struct audio_packet_t*)malloc(sizeof(struct audio_packet_t));
    bzero(ap, sizeof(struct audio_packet_t));
    
    ap->state = state;
    
    ap->mutex = mutex_create();
    
    return ap;
    
}

void audio_packet_set_buffer(struct audio_packet_t* ap, void* in_buffer, size_t in_buffer_size, size_t packet_size) {
    
    packet_size = MAX(in_buffer_size, packet_size);
    
    mutex_lock(ap->mutex);
    
    if (ap->buffer != NULL)
        free(ap->buffer);
    ap->buffer_size = 0;
    
    if (in_buffer != NULL) {
        ap->buffer = malloc(packet_size);
        memcpy(ap->buffer, in_buffer, in_buffer_size);
        bzero(&ap->buffer[in_buffer_size], packet_size - in_buffer_size);
        ap->buffer_size = packet_size;
    } else
        ap->buffer = NULL;
    
    mutex_unlock(ap->mutex);
    
}

size_t audio_packet_get_buffer(struct audio_packet_t* ap, void* buffer, size_t size) {
    
    size_t ret = 0;
    
    mutex_lock(ap->mutex);
    
    ret = MIN(size, ap->buffer_size);
    if (ap->buffer != NULL && buffer != NULL)
        memcpy(buffer, ap->buffer, ret);
    
    mutex_unlock(ap->mutex);
    
    return ret;
    
}

void audio_packet_shift_buffer(struct audio_packet_t* ap, size_t size) {
    
    mutex_lock(ap->mutex);
    
    if (size >= ap->buffer_size) {
        
        if (ap->buffer != NULL)
            free(ap->buffer);
        ap->buffer = NULL;
        ap->buffer_size = 0;
        
    } else {
        
        memcpy(ap->buffer, &((char*)ap->buffer)[size], ap->buffer_size - size);
        ap->buffer_size -= size;
        
    }
    
    mutex_unlock(ap->mutex);
    
}

void audio_packet_destroy(struct audio_packet_t* ap) {
    
    audio_packet_set_buffer(ap, NULL, 0, 0);
    
    mutex_destroy(ap->mutex);
    
    free(ap);
    
}

struct audio_queue_t {
#if DEBUG
    uint32_t non_garbage;
#endif
    audio_queue_received_audio_callback audio_received_callback;
    void* audio_received_callback_ctx;
    audio_output_p output;
    struct audio_packet_t* queue_head;
    struct audio_packet_t* queue_tail;
    decoder_p decoder;
    struct decoder_output_format_t output_format;
    bool output_is_homed;
    double output_homed_at_host_time;
    bool synchronization_enabled;
    double client_server_difference;
    double client_server_difference_history[10];
    int client_server_difference_history_count;
    uint32_t queue_count;
    uint32_t missing_count;
    uint32_t frame_count;
    bool flushed;
    uint16_t flush_last_seq_no;
    double last_known_sample;
    double last_known_sample_time;
    mutex_p mutex;
    condition_p condition;
    bool destroyed;
};

void _audio_queue_add_packet_to_tail(struct audio_queue_t* aq, struct audio_packet_t* packet);
struct audio_packet_t* _audio_queue_add_empty_packet(struct audio_queue_t* aq);
struct audio_packet_t* _audio_queue_pop_packet_from_head(struct audio_queue_t* aq, bool keep_queue_filled);

void _audio_queue_debug_check_queue_consistancy(struct audio_queue_t* aq) {
    
#ifdef DEBUG
    
    assert(aq->non_garbage == 0 /* aq is invalid */);
    assert(aq != NULL /* aq must be non-NUL */);
    assert(!mutex_trylock(aq->mutex) /* Queue mutex must be locked */);
    
    uint32_t queue_count = 0;
    uint32_t missing_count = 0;
    uint32_t frame_count = 0;
    
    if (aq->queue_head != NULL) {
        
        uint16_t c_seq_no = aq->queue_head->seq_no + 1;
        size_t c_sample_time = aq->queue_head->sample_time + (aq->queue_head->buffer_size / aq->output_format.frame_size);
        
        frame_count = ((uint32_t)aq->queue_head->buffer_size / aq->output_format.frame_size);
        
        queue_count++;
        if (aq->queue_head->state != audio_packet_state_complete)
            missing_count++;
        
        LOOP_FROM(current_packet, aq->queue_head->next, next, NULL) {
            
            assert(current_packet->seq_no == c_seq_no /* Package has bad seq no. */);
            assert(current_packet->sample_time == c_sample_time /* Package has bad sample time */);
            
            if (current_packet->state != audio_packet_state_complete)
                missing_count++;
            
            queue_count++;
            
            c_seq_no++;
            c_sample_time += (current_packet->buffer_size / aq->output_format.frame_size);
            frame_count += (current_packet->buffer_size / aq->output_format.frame_size);
            
        }
        
    }
    
    assert(queue_count == aq->queue_count /* Queue count does not match */);
    assert(missing_count == aq->missing_count /* Missing count does not match */);
    assert(frame_count == aq->frame_count /* Queue number of audio frames */);
    
#endif
    
}

void _audio_queue_add_packet_to_tail(struct audio_queue_t* aq, struct audio_packet_t* packet) {
    
    assert(packet != NULL);
    
    packet->previous = aq->queue_tail;
    
    if (aq->queue_tail != NULL) {
        if (packet->sample_time == 0)
            packet->sample_time = aq->queue_tail->sample_time + aq->output_format.frames_per_packet;
        
        aq->queue_tail->next = packet;
    } else
        aq->queue_head = packet;
    
    if (packet->state == audio_packet_state_missing)
        aq->missing_count++;
    
    aq->queue_count++;
    
    aq->queue_tail = packet;
    
    _audio_queue_debug_check_queue_consistancy(aq);
    
    if ((packet->seq_no & 0xFF) == 0xFF)
        log_message(LOG_INFO, "Added package %d to queue", packet->seq_no);
    
    while (aq->queue_count > MAX_QUEUE_COUNT)
        audio_packet_destroy(_audio_queue_pop_packet_from_head(aq, false));
    
}

struct audio_packet_t* _audio_queue_add_empty_packet(struct audio_queue_t* aq) {
    
    struct audio_packet_t* new_packet = audio_packet_create(audio_packet_state_missing);
    
    if (aq->queue_tail != NULL)
        new_packet->seq_no = aq->queue_tail->seq_no + 1;
    
    size_t size = aq->output_format.frame_size * aq->output_format.frames_per_packet;
    char* empty_buffer[size];
    bzero(empty_buffer, size);
    audio_packet_set_buffer(new_packet, empty_buffer, size, size);
    
    aq->frame_count += aq->output_format.frames_per_packet;
    
    _audio_queue_add_packet_to_tail(aq, new_packet);
    
    return new_packet;
    
}

struct audio_packet_t* _audio_queue_pop_packet_from_head(struct audio_queue_t* aq, bool keep_queue_filled) {
    
    struct audio_packet_t* head_packet = aq->queue_head;
    
    if (head_packet != NULL) {
        
        aq->queue_head = head_packet->next;
        if (aq->queue_head != NULL)
            aq->queue_head->previous = NULL;
        else
            aq->queue_tail = NULL;
        
        head_packet->next = NULL;
        head_packet->previous = NULL;
        
        if (head_packet->state >= audio_packet_state_missing)
            aq->missing_count--;
        
        aq->queue_count--;
        aq->frame_count -= head_packet->buffer_size / aq->output_format.frame_size;
        
        if (aq->queue_count < 2 && keep_queue_filled)
            while (aq->queue_count < 2)
                _audio_queue_add_empty_packet(aq);
        
        condition_signal(aq->condition);
        
    }
    
    _audio_queue_debug_check_queue_consistancy(aq);
    
    return head_packet;
    
}

bool _audio_queue_has_available_packet(struct audio_queue_t* aq) {
    
    bool ret = (aq->queue_head != NULL);
    
    if (aq->synchronization_enabled)
        ret = ret && (aq->last_known_sample > 0 && aq->last_known_sample_time > 0);
    else {
        
        ret = ret && (aq->frame_count >= aq->output_format.sample_rate * 2);
        log_message(LOG_INFO, "Queue was synced");
        
    }
    
    return ret;
    
}

double _audio_queue_convert_time(struct audio_queue_t* aq, uint32_t from_sample_time, double from_time, uint32_t to_sample_time) {
    
    return from_time + (((double)to_sample_time - (double)from_sample_time) / (double)aq->output_format.sample_rate);
    
}

void _audio_queue_get_audio_buffer(struct audio_queue_t* aq, void* buffer, size_t size) {
    
    char* buf_write_head = (char*)buffer;
    size_t buf_write_size = size;
    
    size_t out_size = 0;
    
    if (aq->queue_head != NULL) {
        
        while (buf_write_size > 0) {
            
            if (aq->queue_head != NULL) {
                
                struct audio_packet_t* audio_packet = aq->queue_head;
                
                if (audio_packet->state == audio_packet_state_missing) {
                    log_message(LOG_INFO, "Missing package %d made it to playhead", audio_packet->seq_no);
                    audio_output_set_muted(aq->output, true);
                    aq->output_homed_at_host_time = hardware_get_time() + 1.0;
                }
                
                size_t written = audio_packet_get_buffer(audio_packet, buf_write_head, buf_write_size);
                
                aq->frame_count -= written / aq->output_format.frame_size;
                
                audio_packet_shift_buffer(audio_packet, written);
                
                mutex_lock(audio_packet->mutex);
                
                size_t audio_packet_buffer_size = audio_packet->buffer_size;
                
                if (audio_packet_buffer_size > 0) {
                    if (audio_packet->time > 0)
                        audio_packet->time += ((written / aq->output_format.frame_size) / aq->output_format.sample_rate);
                    audio_packet->sample_time += (written / aq->output_format.frame_size);
                }
                
                mutex_unlock(audio_packet->mutex);
                
                buf_write_head = &buf_write_head[written];
                buf_write_size -= written;
                out_size += written;
                
                if (audio_packet_buffer_size == 0)
                    audio_packet_destroy(_audio_queue_pop_packet_from_head(aq, false));
                
            } else
                break;
            
        }
        
    }
    
    if (aq->queue_count == 0)
        aq->last_known_sample = aq->last_known_sample_time = 0;
    
}

void _audio_queue_output_render(audio_output_p ao, void* buffer, size_t size, double host_time, void* ctx) {
    
    struct audio_queue_t* aq = (struct audio_queue_t*)ctx;
    
    mutex_lock(aq->mutex);
    
    if (_audio_queue_has_available_packet(aq)) {
        
        size_t frame_count = size / aq->output_format.frame_size;
        double render_duration = (double)frame_count / (double)aq->output_format.sample_rate;
        double render_start_time = host_time;
        double queue_time = _audio_queue_convert_time(aq, aq->last_known_sample, aq->last_known_sample_time, aq->queue_head->sample_time) + aq->client_server_difference;
        
        size_t data_offset = 0;
        
        if (!aq->output_is_homed) {
            
            if (aq->synchronization_enabled) {
                
                while (queue_time < render_start_time && _audio_queue_has_available_packet(aq)) {
                    audio_packet_destroy(_audio_queue_pop_packet_from_head(aq, false));
                    if (_audio_queue_has_available_packet(aq))
                        queue_time = _audio_queue_convert_time(aq, aq->last_known_sample, aq->last_known_sample_time, aq->queue_head->sample_time) + aq->client_server_difference;
                }
                
                if (_audio_queue_has_available_packet(aq)) {
                    
                    // We calculate for next frame
                    double render_end_time = render_start_time + render_duration;
                    
                    if (queue_time < render_end_time) {
                        
                        double packet_pos = (queue_time - render_start_time) / render_duration;
                        data_offset = floor((double)frame_count * packet_pos) * aq->output_format.frame_size;
                        aq->output_is_homed = true;
                        if (aq->output_homed_at_host_time < host_time)
                            aq->output_homed_at_host_time = host_time;
                        log_message(LOG_INFO, "Output is homed @ %1.5f (%d/%d)", packet_pos, data_offset, size);
                        
                    }
                    
                }
                
            } else
                aq->output_is_homed = true;
            
        }
        
        if (aq->output_is_homed) {
            
            double delay = 0;
            
            if (aq->synchronization_enabled && queue_time > 0) {
                
                delay = queue_time - render_start_time;
                
                if (delay >= -0.1 && delay <= 0.1) {
                    
                    if (aq->output_homed_at_host_time <= host_time)
                        audio_output_set_muted(aq->output, false);
                    audio_output_set_playback_rate(aq->output, MAX(MIN(1.0 - delay, 1.002), 0.998));
                    _audio_queue_get_audio_buffer(aq, &((char*)buffer)[data_offset], size - data_offset);
                    
                } else {
                    
                    aq->output_is_homed = false;
                    aq->output_homed_at_host_time = host_time + 1.0;
                    audio_output_set_muted(aq->output, true);
                    log_message(LOG_INFO, "Resynchronizing output");
                    
                }
                
                if (aq->queue_head && aq->queue_head->seq_no % 100 == 0)
                    log_message(LOG_INFO, "%d packages in queue (%d missing / seq %d / delay %1.3f / playback rate %1.3f)", aq->queue_count, aq->missing_count, aq->queue_head->seq_no, delay, audio_output_get_playback_rate(aq->output));
                
            }
            
        }
        
    } else if (aq->output_is_homed) {
        aq->output_is_homed = false;
        log_message(LOG_INFO, "Output lost synchronization");
    }
    
    mutex_unlock(aq->mutex);
    
}

struct audio_queue_t* audio_queue_create(decoder_p decoder) {
    
    struct audio_queue_t* aq = (struct audio_queue_t*)malloc(sizeof(struct audio_queue_t));
    bzero(aq, sizeof(struct audio_queue_t));
    
    aq->decoder = decoder;
    aq->output_format = decoder_get_output_format(decoder);
    aq->mutex = mutex_create();
    aq->condition = condition_create();
    
    aq->flush_last_seq_no = true;
    aq->synchronization_enabled = true;
  
    aq->output = audio_output_create(aq->output_format);
    audio_output_set_callback(aq->output, _audio_queue_output_render, aq);
    
    return aq;
    
}

void audio_queue_destroy(struct audio_queue_t* aq) {
    
    mutex_lock(aq->mutex);
    
    while (aq->queue_head != NULL) {
        struct audio_packet_t* packet = aq->queue_head;
        aq->queue_head = aq->queue_head->next;
        audio_packet_destroy(packet);
    }
    
    aq->queue_count = aq->missing_count = aq->frame_count = 0;
    
    aq->destroyed = true;
    
    condition_broadcast(aq->condition);
    
    mutex_unlock(aq->mutex);
    
    audio_output_destroy(aq->output);
    
    mutex_destroy(aq->mutex);
    condition_destroy(aq->condition);
    
    free(aq);
    
}

void audio_queue_set_received_audio_callback(struct audio_queue_t* aq, audio_queue_received_audio_callback callback, void* ctx) {
    
    mutex_lock(aq->mutex);
    
    aq->audio_received_callback = callback;
    aq->audio_received_callback_ctx = ctx;
    
    mutex_unlock(aq->mutex);
    
}

void audio_queue_disable_synchronization(struct audio_queue_t* aq) {
    
    mutex_lock(aq->mutex);
    
    log_message(LOG_INFO, "Synchronization is disabled");
    aq->synchronization_enabled = false;
    
    mutex_unlock(aq->mutex);
    
}

void audio_queue_synchronize(struct audio_queue_t* aq, uint32_t current_sample_time, double current_time, uint32_t next_sample_time) {
    
    mutex_lock(aq->mutex);
    
    current_sample_time -= 11025;
    
    if (aq->last_known_sample == 0 && aq->last_known_sample_time == 0) {
        
        while (aq->queue_head && aq->queue_head->sample_time < current_sample_time)
            audio_packet_destroy(_audio_queue_pop_packet_from_head(aq, true));
        
        audio_output_start(aq->output);
        
        log_message(LOG_INFO, "Queue was synced");
        
    }
    
    aq->last_known_sample = current_sample_time;
    aq->last_known_sample_time = current_time;
    
    mutex_unlock(aq->mutex);
    
}

void audio_queue_set_remote_time(struct audio_queue_t* aq, double remote_time) {
    
    mutex_lock(aq->mutex);
    
    double current_time = hardware_get_time();
    
    aq->client_server_difference = 0;
    
    aq->client_server_difference_history_count = MIN(aq->client_server_difference_history_count + 1, CLIENT_SERVER_DIFFERENCE_BACKLOG);
    for (int i = aq->client_server_difference_history_count - 1 ; i > 0 ; i--)
        aq->client_server_difference += (aq->client_server_difference_history[i] = aq->client_server_difference_history[i-1]);
    
    aq->client_server_difference += (aq->client_server_difference_history[0] = current_time - remote_time);
    
    aq->client_server_difference /= (double)aq->client_server_difference_history_count;
    
    log_message(LOG_INFO, "Time difference: %1.5f", aq->client_server_difference);
    
    mutex_unlock(aq->mutex);

}

uint32_t audio_queue_add_packet(struct audio_queue_t* aq, void* encoded_buffer, size_t encoded_buffer_size, uint16_t seq_no, uint32_t sample_time) {
    
    assert(encoded_buffer != NULL && encoded_buffer_size > 0);
    
    int ret = 0;
    
    mutex_lock(aq->mutex);
    
    if (aq->flushed && (seq_no >= aq->flush_last_seq_no || (IS_LOWER(seq_no) && IS_UPPER(aq->flush_last_seq_no)))) {
        aq->flushed = false;
        aq->flush_last_seq_no = 0;
    }
    
    bool package_resend = false;
    
    // Determine if package is a resend package.
    if (aq->queue_head != NULL) {
        
        if (aq->queue_tail->seq_no < aq->queue_head->seq_no)
            package_resend = (seq_no <= aq->queue_tail->seq_no || seq_no >= aq->queue_head->seq_no);
        else
            package_resend = (seq_no >= aq->queue_head->seq_no && seq_no <= aq->queue_tail->seq_no);
        
    }
    
    if (!aq->flushed) {
        
        char* decoded_buffer;
        size_t decoded_buffer_size = encoded_buffer_size;
        
        // Decode if stream is encoded
        if (aq->decoder != NULL) {
            
            decoded_buffer_size = aq->output_format.frame_size * aq->output_format.frames_per_packet;
            decoded_buffer = (char*)malloc(decoded_buffer_size);
            bzero(decoded_buffer, decoded_buffer_size);
            decoded_buffer_size = decoder_decode(aq->decoder, encoded_buffer, encoded_buffer_size, decoded_buffer, decoded_buffer_size);
            
        } else {
            decoded_buffer = (char*)malloc(encoded_buffer_size);
            memcpy(decoded_buffer, encoded_buffer, encoded_buffer_size);
        }
        
        if (aq->queue_head == NULL || !package_resend) {
            
            if (aq->queue_head != NULL) {
                                
                // If sequence number has overflowed
                if (seq_no < aq->queue_tail->seq_no)
                    ret = ((uint32_t)seq_no | (1 << 16)) - aq->queue_tail->seq_no - 1;
                else
                    ret = seq_no - aq->queue_tail->seq_no - 1;
                
                // Late packages can make it here and pose as new packet in far future. Break out if so.
                if (IS_UPPER(ret)) {
                    free(decoded_buffer);
                    mutex_unlock(aq->mutex);
                    return 0;
                }
                
                for (uint32_t i = 0 ; i < ret ; i++)
                    _audio_queue_add_empty_packet(aq);
                
            } else if (!package_resend && aq->audio_received_callback != NULL)
                aq->audio_received_callback(aq, aq->audio_received_callback_ctx);
            
            struct audio_packet_t* new_packet = audio_packet_create(audio_packet_state_complete);
            
            new_packet->sample_time = sample_time;
            new_packet->seq_no = seq_no;
            
            if (aq->queue_count > 0) {
                
                // Compute sample time gap, it will appear if some package is lost.
                size_t ideal_sample_time = sample_time - aq->queue_tail->sample_time;
                size_t ideal_buffer_size = ideal_sample_time * aq->output_format.frame_size;
                
                audio_packet_set_buffer(new_packet, decoded_buffer, decoded_buffer_size, ideal_buffer_size);
                
            } else
                audio_packet_set_buffer(new_packet, decoded_buffer, decoded_buffer_size, decoded_buffer_size);
            
            aq->frame_count += decoded_buffer_size / aq->output_format.frame_size;
            
            _audio_queue_add_packet_to_tail(aq, new_packet);
            
        } else {
            
            bool found = false;
            LOOP_FROM(current_packet, aq->queue_tail, previous, aq->queue_head) {
                
                if (current_packet->seq_no == seq_no) {
                    if (current_packet->state != audio_packet_state_complete) {
                        aq->missing_count--;
                        
                        audio_packet_set_buffer(current_packet, decoded_buffer, decoded_buffer_size, decoded_buffer_size);
                        current_packet->state = audio_packet_state_complete;
                        
                    } else
                        log_message(LOG_INFO, "Packet %d already in queue", seq_no);
                    
                    found = true;
                    break;
                }
                
            }
            
            if (!found)
                log_message(LOG_INFO, "Packet %d came too late", seq_no);
            
        }
        
        free(decoded_buffer);
        
    }
    
    mutex_unlock(aq->mutex);
    
    return ret;
    
}

struct audio_queue_missing_packet_window audio_queue_get_next_missing_window(struct audio_queue_t* aq) {
    
    struct audio_queue_missing_packet_window ret;
    
    ret.seq_no = 0;
    ret.packet_count = 0;
    
    mutex_lock(aq->mutex);
    
    if (aq->missing_count < aq->queue_count / 2) {
        
        uint32_t queue_pos = 0; // Stop when 3/4 of queue has been searched
        for (struct audio_packet_t* current_packet = aq->queue_head ; current_packet != NULL && queue_pos < aq->queue_count - aq->queue_count / 4 ; current_packet = current_packet->next) {
            
            if (current_packet->state == audio_packet_state_missing) {
                
                ret.seq_no = current_packet->seq_no;
                
                for (struct audio_packet_t* missing_packet = current_packet ; missing_packet->state == audio_packet_state_missing ; missing_packet = missing_packet->next) {
                    ret.packet_count++;
                    missing_packet->state = audio_packet_state_requested;
                }
                
                break;
                
            }
            
            queue_pos++;
            
        }
        
    }
    
    mutex_unlock(aq->mutex);
    
    return ret;
    
}

void audio_queue_start(struct audio_queue_t* aq) {
    
    mutex_lock(aq->mutex);
    
    aq->flushed = false;
    
    audio_output_start(aq->output);
    
    mutex_unlock(aq->mutex);
    
}

void audio_queue_flush(struct audio_queue_t* aq, uint16_t last_seq_no) {
    
    mutex_lock(aq->mutex);
    
    while (aq->queue_head != NULL)
        audio_packet_destroy(_audio_queue_pop_packet_from_head(aq, false));
    
    aq->flushed = true;
    aq->flush_last_seq_no = last_seq_no;
    aq->last_known_sample = aq->last_known_sample_time = 0;
    
    audio_output_flush(aq->output);
    decoder_reset(aq->decoder),
    
    mutex_unlock(aq->mutex);
    
    log_message(LOG_INFO, "Queue flushed");
        
}

bool audio_queue_wait_for_space(struct audio_queue_t* aq) {
    
    mutex_lock(aq->mutex);
    
    audio_output_stop(aq->output);
    
    while ((aq->synchronization_enabled && aq->queue_count == MAX_QUEUE_COUNT) || (!aq->synchronization_enabled && (aq->frame_count >= aq->output_format.sample_rate * 4)))
        condition_wait(aq->condition, aq->mutex);
    
    bool ret = !aq->destroyed;
    
    mutex_unlock(aq->mutex);
    
    return ret;
    
}

audio_output_p audio_queue_get_output(struct audio_queue_t* aq) {
    
    return aq->output;
    
}

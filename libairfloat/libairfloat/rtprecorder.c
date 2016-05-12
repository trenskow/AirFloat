//
//  rtprecorder.c
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
#include <math.h>

#include "crypt.h"
#include "mutex.h"
#include "log.h"
#include "condition.h"
#include "thread.h"
#include "hardware.h"
#include "rtpsocket.h"
#include "audioqueue.h"

#include "rtprecorder.h"

#define RTP_SOURCE                      0x0f
#define RTP_EXTENSION                   0x10
#define RTP_PAYLOAD_TYPE                0x7f
#define RTP_MARKER                      0x80

#define RTP_TIMING_REQUEST              0x52
#define RTP_TIMING_RESPONSE             0x53
#define RTP_SYNC                        0x54
#define RTP_RANGE_RESEND_REQUEST        0x55
#define RTP_AUDIO_RESEND_DATA           0x56
#define RTP_AUDIO_DATA                  0x60

#define NTP_UNIXEPOCH 0x83aa7e80
#define NTP_FRACTION 0xFFFFFFFF

struct ntp_time {
    uint32_t integer;
    uint32_t fraction;
};

struct ntp_time _ntp_time_from_hardware_time(double time) {
    
    struct ntp_time ret;
    bzero(&ret, sizeof(struct ntp_time));
    
    double integer = floor(time);
    uint32_t i = integer;
    i += NTP_UNIXEPOCH;
    ret.integer = htonl(i);
    ret.fraction = (time - integer) * (double)NTP_FRACTION;
    
    ret.fraction = htonl(ret.fraction);
    
    return ret;
    
}

double _ntp_time_to_hardware_time(struct ntp_time time) {
    
    uint32_t i = ntohl(time.integer);
    i -= NTP_UNIXEPOCH;
    return i + (ntohl(time.fraction) / (double)NTP_FRACTION);
    
}

struct rtp_packet_t {
    uint16_t seq_num;
    bool extension;
    uint8_t source;
    uint8_t payload_type;
    bool marker;
    void* packet_data;
    size_t packet_data_size;
};

struct rtp_timing_packet_t {
    uint8_t a;
    uint8_t b;
    uint16_t seq_num;
    uint32_t _padding;
    struct ntp_time reference_time;
    struct ntp_time received_time;
    struct ntp_time send_time;
};

#define RTP_TIMING_PACKET_SIZE 32

struct rtp_resent_packet_t {
    uint8_t a;
    uint8_t b;
    uint16_t seq_num; /* Not used */
    uint16_t missed_seq;
    uint16_t count;
};

#define RTP_RESEND_PACKET_SIZE 8

struct rtp_packet_t _rtp_header_read(const void* buffer, size_t size) {
    
    assert(buffer != NULL && size > 0);
    
    struct rtp_packet_t ret;
    memset(&ret, 0, sizeof(struct rtp_packet_t));
    
    char a = ((const char*)buffer)[0];
    char b = ((const char*)buffer)[1];
    
    ret.seq_num = ntohs(((uint16_t*)buffer)[1]);
    
    ret.extension = (bool)(a & RTP_EXTENSION);
    ret.source = a & RTP_SOURCE;
    ret.payload_type = b & RTP_PAYLOAD_TYPE;
    ret.marker = b & RTP_MARKER;
    
    ret.packet_data = (void*)buffer + 4;
    ret.packet_data_size = size - 4;
    
    return ret;
    
}

struct rtp_recorder_t {
    crypt_aes_p crypt;
    audio_queue_p audio_queue;
    mutex_p timer_mutex;
    condition_p timer_cond;
    thread_p synchronization_thread;
    uint32_t initial_time_response_count;
    rtp_socket_p streaming_socket;
    rtp_socket_p timing_socket;
    rtp_socket_p control_socket;
    struct sockaddr* remote_timing_end_point;
    struct sockaddr* remote_control_end_point;
    uint16_t emulated_seq_no;
    
    rtp_recorder_updated_track_position_callback updated_track_position_callback;
    void* updated_track_position_callback_ctx;
};

void _rtp_recorder_process_timing_packet(struct rtp_recorder_t* rr, struct rtp_packet_t* packet) {
    
    assert(packet != NULL);
    
    double current_time = hardware_get_time();
    double reference_time = _ntp_time_to_hardware_time(*(struct ntp_time*)(packet->packet_data + 4));
    double received_time = _ntp_time_to_hardware_time(*(struct ntp_time*)(packet->packet_data + 12));
    double send_time = _ntp_time_to_hardware_time(*(struct ntp_time*)(packet->packet_data + 20));
    
    double delay = ((current_time - reference_time) - (send_time - received_time)) / 2;
    double client_time = received_time + (send_time - received_time) + delay;
    
    log_message(LOG_INFO, "Client time is %1.6f (peer delay: %1.6f)", client_time, delay);
    
    audio_queue_set_remote_time(rr->audio_queue, client_time);
    
    rr->initial_time_response_count++;
    
    if (rr->initial_time_response_count == 2)
        condition_signal(rr->timer_cond);
    
}

void _rtp_recorder_send_timing_request(struct rtp_recorder_t* rr) {
    
    struct rtp_timing_packet_t pckt;
    bzero(&pckt, sizeof(struct rtp_timing_packet_t));
    
    pckt.a = 0x80;
    pckt.b = RTP_TIMING_REQUEST | ~RTP_PAYLOAD_TYPE;
    
    pckt.seq_num = 0x0700;
    
    double send_time = hardware_get_time();
    pckt.send_time = _ntp_time_from_hardware_time(send_time);
    
    rtp_socket_send_to(rr->timing_socket, rr->remote_timing_end_point, &pckt, RTP_TIMING_PACKET_SIZE);
    
    log_message(LOG_INFO, "Timing synchronization request sent (@ %1.6f)", send_time);
    
}

void _rtp_recorder_synchronization_loop(void* ctx) {
    
    thread_set_name("Synchronization loop");
    
    struct rtp_recorder_t* rr = (struct rtp_recorder_t*)ctx;
    
    mutex_lock(rr->timer_mutex);
    
    while (condition_times_wait(rr->timer_cond, rr->timer_mutex, 2000))
        _rtp_recorder_send_timing_request(rr);
    
    mutex_unlock(rr->timer_mutex);
    
}

void _rtp_recorder_process_sync_packet(struct rtp_recorder_t* rr, struct rtp_packet_t* packet) {
    
    assert(packet != NULL);
    
    uint32_t current_rtp_time = ntohl(*((uint32_t*)packet->packet_data));
    double current_time = _ntp_time_to_hardware_time(*(struct ntp_time*)(packet->packet_data + 4));
    uint32_t next_rtp_time = ntohl(*((uint32_t*)packet->packet_data + 12));
    
    log_message(LOG_INFO, "Sync packet (Playhead frame: %u - current time: %1.6f - next frame: %u)", current_rtp_time, current_time, next_rtp_time);
    if (rr->updated_track_position_callback != NULL) {
        rr->updated_track_position_callback(rr, current_rtp_time, rr->updated_track_position_callback_ctx);
    }
    
    audio_queue_synchronize(rr->audio_queue, current_rtp_time, current_time, next_rtp_time);
    
}

void _rtp_recorder_send_resend_request(struct rtp_recorder_t* rr, uint16_t seq_num, uint16_t count) {
    
    assert(count > 0);
    
    struct rtp_resent_packet_t pckt;
    
    memset(&pckt, 0, sizeof(struct rtp_resent_packet_t));
    
    pckt.a = 0x80;
    pckt.b = RTP_RANGE_RESEND_REQUEST | ~RTP_PAYLOAD_TYPE;
    pckt.seq_num = pckt.count = htons(count);
    pckt.missed_seq = htons(seq_num);
    
    rtp_socket_send_to(rr->control_socket, rr->remote_control_end_point, &pckt, RTP_RESEND_PACKET_SIZE);
    
    log_message(LOG_INFO, "Requested packet resend (seq: %d / count %d)", seq_num, count);
    
}

void _rtp_recorder_process_audio_packet(struct rtp_recorder_t* rr, struct rtp_packet_t* packet) {
    
    assert(packet != NULL);
    
    uint32_t rtp_time = ntohl(*(uint32_t*)packet->packet_data);
    uint16_t c_seq = packet->seq_num;
    
    char* packet_audio_data = packet->packet_data + 8;
    size_t len = packet->packet_data_size - 8;
    
    char* decoded_audio_data = (char*)malloc(len);
    
    if (rr->crypt != NULL)
        len = crypt_aes_decrypt(rr->crypt, packet_audio_data, len, decoded_audio_data, len);
    else
        memcpy(decoded_audio_data, packet_audio_data, len);
    
    uint16_t missing_count = audio_queue_add_packet(rr->audio_queue, decoded_audio_data, len, c_seq, rtp_time);
    if (missing_count > 0)
        _rtp_recorder_send_resend_request(rr, c_seq - missing_count, missing_count);
    
    free(decoded_audio_data);
    
    struct audio_queue_missing_packet_window next_missing_window = audio_queue_get_next_missing_window(rr->audio_queue);
    if (next_missing_window.seq_no > 0 || next_missing_window.packet_count > 0)
        _rtp_recorder_send_resend_request(rr, next_missing_window.seq_no, next_missing_window.packet_count);
    
}

size_t _rtp_recorder_socket_data_received_airtunes_v1(struct rtp_recorder_t* rr, rtp_socket_p rtp_socket, socket_p socket, const void* buffer, size_t size) {
        
    size_t read = 0;
    
    while (read < size) {
        
        if (size < 4)
            break;
        
        uint16_t packet_size = ntohs(*((uint16_t*)buffer + read + 2));
        if (size - read < packet_size + 4)
            break;
        
        read += 4;
        
        if (((char*)buffer)[read] == '\xf0' && ((char*)buffer)[read + 1] == '\xff') {
            
            struct rtp_packet_t packet = _rtp_header_read(&((char*)&buffer)[read], packet_size);
            
            packet.seq_num = rr->emulated_seq_no++;
            
            _rtp_recorder_process_audio_packet(rr, &packet);
            
        }
        
        read += packet_size;
        
        assert(read <= size);
        
    }
    
    return read;
    
}

size_t _rtp_recorder_socket_data_received_airtunes_v2(struct rtp_recorder_t* rr, rtp_socket_p rtp_socket, socket_p socket, const void* buffer, size_t size) {
    
    struct rtp_packet_t packet = _rtp_header_read(buffer, size);
    
    switch (packet.payload_type) {
        case RTP_TIMING_RESPONSE:
            _rtp_recorder_process_timing_packet(rr, &packet);
            break;
        case RTP_SYNC:
            _rtp_recorder_process_sync_packet(rr, &packet);
            break;
        case RTP_AUDIO_RESEND_DATA:
            packet = _rtp_header_read(&((char*)buffer)[4], size - 4);
            log_message(LOG_INFO, "Received missing packet %d", packet.seq_num);
            break;
        case RTP_AUDIO_DATA:
            if (packet.packet_data_size > 0)
                _rtp_recorder_process_audio_packet(rr, &packet);
            break;
        default:
            log_message(LOG_ERROR, "Received unknown packet");
            break;
    }
    
    return size;
    
}

size_t _rtp_recorder_socket_data_received_callback(rtp_socket_p rtp_socket, socket_p socket, const void* buffer, size_t size, void* ctx) {
    
    struct rtp_recorder_t* rr = (struct rtp_recorder_t*)ctx;
    
    if (socket_is_udp(socket))
        return _rtp_recorder_socket_data_received_airtunes_v2(rr, rtp_socket, socket, buffer, size);
    
    return _rtp_recorder_socket_data_received_airtunes_v1(rr, rtp_socket, socket, buffer, size);
    
}

rtp_socket_p _rtp_recorder_create_socket(struct rtp_recorder_t* rr, const char* name, struct sockaddr* local_end_point, struct sockaddr* remote_end_point) {
    
    rtp_socket_p ret = rtp_socket_create(name, remote_end_point);
    
    unsigned short p;
    for (p = 6000 ; p < 6100 ; p++) {
        struct sockaddr* ep = sockaddr_copy(local_end_point);
        sockaddr_set_port(ep, p);
        if (rtp_socket_setup(ret, ep)) {
            rtp_socket_set_data_received_callback(ret, _rtp_recorder_socket_data_received_callback, rr);
            log_message(LOG_INFO, "Setup socket on port %u", p);
            sockaddr_destroy(ep);
            return ret;
        }
        sockaddr_destroy(ep);
    }
    
    log_message(LOG_ERROR, "Unable to bind socket.");
    
    rtp_socket_destroy(ret);
    
    return NULL;
    
}

struct rtp_recorder_t* rtp_recorder_create(crypt_aes_p crypt, audio_queue_p audio_queue, struct sockaddr* local_end_point, struct sockaddr* remote_end_point, uint16_t remote_control_port, uint16_t remote_timing_port) {
    
    struct rtp_recorder_t* rr = (struct rtp_recorder_t*)malloc(sizeof(struct rtp_recorder_t));
    bzero(rr, sizeof(struct rtp_recorder_t));
    
    rr->crypt = crypt;
    rr->audio_queue = audio_queue;
    
    rr->timer_mutex = mutex_create();
    rr->timer_cond = condition_create();
    
    rr->remote_control_end_point = sockaddr_copy(remote_end_point);
    rr->remote_timing_end_point = sockaddr_copy(remote_end_point);
    
    sockaddr_set_port(rr->remote_control_end_point, remote_control_port);
    sockaddr_set_port(rr->remote_timing_end_point, remote_timing_port);
    
    rr->streaming_socket = _rtp_recorder_create_socket(rr, "Straming socket", local_end_point, remote_end_point);
    rr->control_socket = _rtp_recorder_create_socket(rr, "Control socket", local_end_point, remote_end_point);
    rr->timing_socket = _rtp_recorder_create_socket(rr, "Timing socket", local_end_point, remote_end_point);
    
    rr->updated_track_position_callback = NULL;
    rr->updated_track_position_callback_ctx = NULL;
    
    return rr;
    
}

void rtp_recorder_destroy(struct rtp_recorder_t* rr) {
    
    mutex_lock(rr->timer_mutex);
    
    if (rr->synchronization_thread != NULL) {
        condition_signal(rr->timer_cond);
        mutex_unlock(rr->timer_mutex);
        thread_join(rr->synchronization_thread);
        mutex_lock(rr->timer_mutex);
        thread_destroy(rr->synchronization_thread);
        rr->synchronization_thread = NULL;
    }
    
    mutex_unlock(rr->timer_mutex);
    
    rtp_socket_destroy(rr->streaming_socket);
    rtp_socket_destroy(rr->control_socket);
    rtp_socket_destroy(rr->timing_socket);
    
    sockaddr_destroy(rr->remote_control_end_point);
    sockaddr_destroy(rr->remote_timing_end_point);
    
    mutex_destroy(rr->timer_mutex);
    condition_destroy(rr->timer_cond);
    
    free(rr);
    
}

bool rtp_recorder_start(struct rtp_recorder_t* rr) {
    
    bool complete = true;
    
    if (rr->initial_time_response_count < 3) {
        
        _rtp_recorder_send_timing_request(rr);
        _rtp_recorder_send_timing_request(rr);
        _rtp_recorder_send_timing_request(rr);
        
        log_message(LOG_INFO, "Waiting for synchronization");
        
        mutex_lock(rr->timer_mutex);
        
        complete = !condition_times_wait(rr->timer_cond, rr->timer_mutex, 5000);
        
        if (!complete)
            log_message(LOG_INFO, "Initial time synchronization incomplete");
        else {
            rr->synchronization_thread = thread_create_a(_rtp_recorder_synchronization_loop, rr);
            log_message(LOG_INFO, "Initial time synchronization complete");
        }
        
        mutex_unlock(rr->timer_mutex);
        
    }
    
    return complete;
    
}

uint16_t rtp_recorder_get_streaming_port(struct rtp_recorder_t* rr) {
    
    return rtp_socket_get_local_port(rr->streaming_socket);
    
}

uint16_t rtp_recorder_get_control_port(struct rtp_recorder_t* rr) {
    
    return rtp_socket_get_local_port(rr->control_socket);
    
}

uint16_t rtp_recorder_get_timing_port(struct rtp_recorder_t* rr) {
    
    return rtp_socket_get_local_port(rr->timing_socket);
    
}

void rtp_recorder_set_updated_track_position_callback(struct rtp_recorder_t* rr, rtp_recorder_updated_track_position_callback callback, void* ctx) {
    rr->updated_track_position_callback = callback;
    rr->updated_track_position_callback_ctx = ctx;
}

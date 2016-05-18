//
//  rtprecorder.h
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

#ifndef _rtprecorder_h
#define _rtprecorder_h

#include "crypt.h"
#include "audioqueue.h"
#include "sockaddr.h"

typedef struct rtp_recorder_t *rtp_recorder_p;

rtp_recorder_p rtp_recorder_create(crypt_aes_p crypt, audio_queue_p audio_queue, struct sockaddr* local_end_point, struct sockaddr* remote_end_point, uint16_t remote_control_port, uint16_t remote_timing_port);
void rtp_recorder_destroy(rtp_recorder_p rr);
bool rtp_recorder_start(rtp_recorder_p rr);
uint16_t rtp_recorder_get_streaming_port(rtp_recorder_p rr);
uint16_t rtp_recorder_get_control_port(rtp_recorder_p rr);
uint16_t rtp_recorder_get_timing_port(rtp_recorder_p rr);

typedef void(*rtp_recorder_updated_track_position_callback)(rtp_recorder_p rr, unsigned int curr, void* ctx);
void rtp_recorder_set_updated_track_position_callback(rtp_recorder_p rr, rtp_recorder_updated_track_position_callback callback, void* ctx);

#endif

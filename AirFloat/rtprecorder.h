//
//  rtprecorder.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
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

#endif

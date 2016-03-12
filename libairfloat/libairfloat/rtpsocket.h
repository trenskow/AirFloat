//
//  rtpsocket.h
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

#ifndef _rtpsocket_h
#define _rtpsocket_h

#include <stdint.h>
#include <stdbool.h>

#include "mutex.h"
#include "thread.h"
#include "sockaddr.h"
#include "socket.h"

typedef struct rtp_socket_t *rtp_socket_p;

typedef size_t(*rtp_socket_data_received_callback)(rtp_socket_p rtp_socket, socket_p socket, const void* buffer, size_t size, void* ctx);

rtp_socket_p rtp_socket_create(const char* name, struct sockaddr* allowed_remote_end_point);
void rtp_socket_destroy(rtp_socket_p rs);
bool rtp_socket_setup(rtp_socket_p rs, struct sockaddr* local_end_point);
void rtp_socket_set_data_received_callback(rtp_socket_p rs, rtp_socket_data_received_callback callback, void* ctx);
void rtp_socket_send_to(rtp_socket_p rs, struct sockaddr* dst, const void* buffer, uint32_t size);
uint16_t rtp_socket_get_local_port(rtp_socket_p rs);

#endif

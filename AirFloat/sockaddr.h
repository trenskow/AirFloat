//
//  sockaddr.h
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

#ifndef __SOCKETENDPOINT_H
#define __SOCKETENDPOINT_H

#include <sys/socket.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    
    sockaddr_type_inet_4 = 1 << 1,
    sockaddr_type_inet_6 = 1 << 2
    
} sockaddr_type;

struct sockaddr* sockaddr_create(const char* host, uint16_t port, sockaddr_type version, uint32_t scope_id);
void sockaddr_destroy(struct sockaddr* addr);
struct sockaddr* sockaddr_copy(struct sockaddr* addr);
bool sockaddr_equals(struct sockaddr* addr1, struct sockaddr* addr2);
bool sockaddr_equals_host(struct sockaddr* addr1, struct sockaddr* addr2);
const char* sockaddr_get_host(struct sockaddr* addr);
uint16_t sockaddr_get_port(struct sockaddr* addr);
void sockaddr_set_port(struct sockaddr* addr, uint16_t new_port);
bool sockaddr_is_ipv6(struct sockaddr* addr);

#endif

//
//  endpoint.h
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

#include "object.h"

typedef struct endpoint_t *endpoint_p;

typedef enum {
    endpoint_type_inet_4 = 1 << 1,
    endpoint_type_inet_6 = 1 << 2
} endpoint_type;

endpoint_p endpoint_create(const char* host, uint16_t port, endpoint_type version, uint32_t scope_id);
endpoint_p endpoint_create_sockaddr(struct sockaddr* addr);
endpoint_p endpoint_copy(endpoint_p addr);
bool endpoint_equals(endpoint_p addr1, endpoint_p addr2);
bool endpoint_equals_host(endpoint_p addr1, endpoint_p addr2);
const char* endpoint_get_host(endpoint_p addr);
uint16_t endpoint_get_port(endpoint_p addr);
void endpoint_set_port(endpoint_p addr, uint16_t new_port);
bool endpoint_is_ipv6(endpoint_p addr);
void endpoint_copy_sockaddr(endpoint_p endpoint, struct sockaddr* addr);
const struct sockaddr* endpoint_get_sockaddr(endpoint_p endpoint);

#endif

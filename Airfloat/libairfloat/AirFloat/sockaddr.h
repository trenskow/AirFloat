//
//  SocketEndPoint.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
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

//
//  Socket.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __SOCKET_H
#define __SOCKET_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>

typedef struct socket_t *socket_p;

socket_p socket_create(bool is_udp);
void socket_destroy(socket_p s);
bool socket_bind(socket_p s, struct sockaddr* end_point);
bool socket_listen(socket_p s);
socket_p socket_accept(socket_p s);
int64_t socket_receive(socket_p s, void* buffer, uint32_t size);
int64_t socket_send(socket_p s, const void* buffer, uint32_t size);
int64_t socket_send_to(socket_p s, struct sockaddr* end_point, const void* buffer, uint32_t size);
void socket_close(socket_p s);
struct sockaddr* socket_get_local_end_point(socket_p s);
struct sockaddr* socket_get_remote_end_point(socket_p s);
bool socket_is_udp(socket_p s);

#endif

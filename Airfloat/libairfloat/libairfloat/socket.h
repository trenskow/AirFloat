//
//  Socket.h
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

#ifndef __SOCKET_H
#define __SOCKET_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>

typedef struct socket_t *socket_p;

typedef bool(*socket_accept_callback)(socket_p socket, socket_p new_socket, void* ctx);
typedef void(*socket_connected_callback)(socket_p socket, void* ctx);
typedef void(*socket_connect_failed_callback)(socket_p socket, void* ctx);
typedef ssize_t(*socket_receive_callback)(socket_p socket, const void* data, size_t data_size, struct sockaddr* remote_end_point, void* ctx);
typedef void(*socket_closed_callback)(socket_p socket, void* ctx);

socket_p socket_create(const char* name, bool is_udp);
void socket_destroy(socket_p s);
bool socket_bind(socket_p s, struct sockaddr* end_point);
void socket_connect(socket_p s, struct sockaddr* end_point);
void socket_set_accept_callback(socket_p s, socket_accept_callback callback, void* ctx);
void socket_set_connected_callback(socket_p s, socket_connected_callback callback, void* ctx);
void socket_set_connect_failed_callback(socket_p s, socket_connect_failed_callback callback, void* ctx);
void socket_set_receive_callback(socket_p s, socket_receive_callback callback, void* ctx);
void socket_set_closed_callback(socket_p s, socket_closed_callback callback, void* ctx);
ssize_t socket_send(socket_p s, const void* buffer, size_t size);
ssize_t socket_send_to(socket_p s, struct sockaddr* end_point, const void* buffer, size_t size);
void socket_close(socket_p s);
struct sockaddr* socket_get_local_end_point(socket_p s);
struct sockaddr* socket_get_remote_end_point(socket_p s);
bool socket_is_udp(socket_p s);
bool socket_is_connected(socket_p s);

#endif

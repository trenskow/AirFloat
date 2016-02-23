//
//  webserver.h
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

#ifndef _webserver_h
#define _webserver_h

#include <stdint.h>

#include "thread.h"
#include "sockaddr.h"
#include "socket.h"
#include "webserverconnection.h"

#ifndef _wsp
typedef struct web_server_t *web_server_p;
#define _wsp
#endif

typedef bool(*web_server_accept_callback)(web_server_p server, web_server_connection_p connection, void* ctx);

web_server_p web_server_create(sockaddr_type socket_types);
void web_server_destroy(web_server_p ws);
bool web_server_start(web_server_p ws, uint16_t port);
bool web_server_is_running(web_server_p ws);
void web_server_stop(web_server_p ws);
uint32_t web_server_get_connection_count(web_server_p ws);
struct sockaddr* web_server_get_local_end_point(web_server_p ws, sockaddr_type socket_type);
void web_server_set_accept_callback(web_server_p ws, web_server_accept_callback accept_callback, void* ctx);

#endif

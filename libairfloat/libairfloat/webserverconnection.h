//
//  webserverconnection.h
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

#ifndef __webserverconnection_h
#define __webserverconnection_h

#include <stdbool.h>

#include "thread.h"
#include "socket.h"
#include "sockaddr.h"
#include "webrequest.h"
#include "webresponse.h"

#define READ_SIZE 16384

#ifndef _wsp
typedef struct web_server_t *web_server_p;
#define _wsp
#endif

typedef struct web_server_connection_t *web_server_connection_p;

typedef void(*web_server_connection_request_callback)(web_server_connection_p connection, web_request_p request, void* ctx);
typedef void(*web_server_connection_closed_callback)(web_server_connection_p connection, void* ctx);

web_server_connection_p web_server_connection_create(socket_p socket, web_server_p server);
void web_server_connection_destroy(web_server_connection_p connection);
void web_server_connection_set_request_callback(web_server_connection_p wc, web_server_connection_request_callback request_callback, void* ctx);
void web_server_connection_set_closed_callback(web_server_connection_p wc, web_server_connection_closed_callback closed_callback, void* ctx);
void web_server_connection_send_response(web_server_connection_p wc, web_response_p response, const char* protocol, bool close_after_send);
bool web_server_connection_is_connected(web_server_connection_p wc);
void web_server_connection_take_off(struct web_server_connection_t* wc);
void web_server_connection_close(web_server_connection_p wc);
struct sockaddr* web_server_connection_get_local_end_point(web_server_connection_p wc);
struct sockaddr* web_server_connection_get_remote_end_point(web_server_connection_p wc);
const char* web_server_connection_get_host(web_server_connection_p wc);
uint16_t web_server_connection_get_port(web_server_connection_p wc);

#endif

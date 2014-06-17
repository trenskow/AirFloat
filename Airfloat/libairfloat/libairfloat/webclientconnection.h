//
//  webclientconnection.h
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

#ifndef _webclientconnection_h
#define _webclientconnection_h

#include "webrequest.h"
#include "webresponse.h"
#include "sockaddr.h"

typedef struct web_client_connection_t *web_client_connection_p;

typedef void(*web_client_connection_connected_callback)(web_client_connection_p connection, void* ctx);
typedef void(*web_client_connection_connect_failed_callback)(web_client_connection_p connection, void* ctx);
typedef void(*web_client_connection_response_received_callback)(web_client_connection_p connection, web_request_p request, web_response_p response, void* ctx);
typedef void(*web_client_connection_disconnected_callback)(web_client_connection_p connection, void* ctx);

web_client_connection_p web_client_connection_create();
void web_client_connection_destroy(web_client_connection_p wc);
void web_client_connection_set_connected_callback(web_client_connection_p wc, web_client_connection_connected_callback callback, void* ctx);
void web_client_connection_set_connect_failed_callback(web_client_connection_p wc, web_client_connection_connect_failed_callback callback, void* ctx);
void web_client_connection_set_response_received_callback(web_client_connection_p wc, web_client_connection_response_received_callback callback, void* ctx);
void web_client_connection_set_disconneced_callback(web_client_connection_p wc, web_client_connection_disconnected_callback callback, void* ctx);
void web_client_connection_connect(web_client_connection_p wc, struct sockaddr* end_point);
bool web_client_connection_is_connected(web_client_connection_p wc);
void web_client_connection_send_request(web_client_connection_p wc, web_request_p request);

#endif

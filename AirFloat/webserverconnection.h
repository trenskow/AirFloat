//
//  webserverconnection.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
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

#endif

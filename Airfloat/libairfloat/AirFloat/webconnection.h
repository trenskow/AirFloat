//
//  webconnection.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __webconnection_h
#define __webconnection_h

#include <stdbool.h>

#include "thread.h"
#include "socket.h"
#include "sockaddr.h"
#include "webrequest.h"

#define READ_SIZE 16384

#ifndef _wsp
typedef struct web_server_t *web_server_p;
#define _wsp
#endif

typedef struct web_connection_t *web_connection_p;

typedef void(*web_connection_request_callback)(web_connection_p connection, web_request_p request, void* ctx);
typedef void(*web_connection_closed_callback)(web_connection_p connection, void* ctx);

web_connection_p web_connection_create(socket_p socket, web_server_p server);
void web_connection_destroy(web_connection_p connection);
void web_connection_set_request_callback(web_connection_p wc, web_connection_request_callback request_callback, void* ctx);
void web_connection_set_close_callback(web_connection_p wc, web_connection_closed_callback closed_callback, void* ctx);
bool web_connection_is_connected(web_connection_p wc);
void web_connection_close(web_connection_p wc);
void web_connection_wait_close(web_connection_p wc);
struct sockaddr* web_connection_get_local_end_point(web_connection_p wc);
struct sockaddr* web_connection_get_remote_end_point(web_connection_p wc);

#endif

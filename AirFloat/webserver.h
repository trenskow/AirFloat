//
//  webserver.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
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

//
//  webclientconnection.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/30/13.
//
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

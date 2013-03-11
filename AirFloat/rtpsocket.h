//
//  rtpsocket.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/20/13.
//
//

#ifndef _rtpsocket_h
#define _rtpsocket_h

#include <stdint.h>
#include <stdbool.h>

#include "mutex.h"
#include "thread.h"
#include "sockaddr.h"
#include "socket.h"

typedef struct rtp_socket_t *rtp_socket_p;

typedef size_t(*rtp_socket_data_received_callback)(rtp_socket_p rtp_socket, socket_p socket, const void* buffer, size_t size, void* ctx);

rtp_socket_p rtp_socket_create(const char* name, struct sockaddr* allowed_remote_end_point);
void rtp_socket_destroy(rtp_socket_p rs);
bool rtp_socket_setup(rtp_socket_p rs, struct sockaddr* local_end_point);
void rtp_socket_set_data_received_callback(rtp_socket_p rs, rtp_socket_data_received_callback callback, void* ctx);
void rtp_socket_send_to(rtp_socket_p rs, struct sockaddr* dst, const void* buffer, uint32_t size);
uint16_t rtp_socket_get_local_port(rtp_socket_p rs);

#endif

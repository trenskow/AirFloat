//
//  raopserver.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
//

#ifndef _raopserver_h
#define _raopserver_h

#include <stdint.h>
#include <stdbool.h>

typedef struct raop_server_t *raop_server_p;

extern const char* raop_server_localhost_refused_notification;

raop_server_p raop_server_create();
void raop_server_destroy(raop_server_p rs);
bool raop_server_start(raop_server_p rs, uint16_t port);
bool raop_server_is_running(raop_server_p rs);
bool raop_server_is_recording(raop_server_p rs);
void raop_server_stop(raop_server_p rs);

#endif

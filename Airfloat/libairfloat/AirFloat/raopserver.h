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

struct raop_server_settings_t {
    const char* name;
    const char* password;
};

typedef struct raop_server_t *raop_server_p;

#ifndef _rsp
typedef struct raop_session_t *raop_session_p;
#define _rsp
#endif

typedef void(*raop_server_new_session_callback)(raop_server_p server, raop_session_p new_session, void* ctx);

raop_server_p raop_server_create(struct raop_server_settings_t settings);
void raop_server_destroy(raop_server_p rs);
bool raop_server_start(raop_server_p rs, uint16_t port);
bool raop_server_is_running(raop_server_p rs);
bool raop_server_is_recording(raop_server_p rs);
struct raop_server_settings_t raop_server_get_settings(raop_server_p rs);
void raop_server_set_settings(raop_server_p rs, struct raop_server_settings_t settings);
void raop_server_stop(raop_server_p rs);
void raop_server_set_new_session_callback(raop_server_p rs, raop_server_new_session_callback new_session_callback, void* ctx);

#endif

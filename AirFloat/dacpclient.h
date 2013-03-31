//
//  dacpclient.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
//

#ifndef _dacpclient_h
#define _dacpclient_h

#include "sockaddr.h"

enum dacp_client_playback_state {
    dacp_client_playback_state_stopped = 2,
    dacp_client_playback_state_paused,
    dacp_client_playback_state_playing
};

typedef struct dacp_client_t *dacp_client_p;

typedef void(*dacp_client_controls_became_available_callback)(dacp_client_p client, void* ctx);
typedef void(*dacp_client_controls_became_unavailable_callback)(dacp_client_p client, void* ctx);
typedef void(*dacp_client_playback_state_changed_callback)(dacp_client_p client, enum dacp_client_playback_state state, void* ctx);

dacp_client_p dacp_client_create(struct sockaddr* end_point, const char* identifier, const char* active_remote);
void dacp_client_destroy(dacp_client_p dc);
void dacp_client_set_controls_became_available_callback(dacp_client_p dc, dacp_client_controls_became_available_callback callback, void* ctx);
void dacp_client_set_controls_became_unavailable_callback(dacp_client_p dc, dacp_client_controls_became_unavailable_callback callback, void* ctx);
void dacp_client_set_playback_state_changed_callback(dacp_client_p dc, dacp_client_playback_state_changed_callback callback, void* ctx);
bool dacp_client_is_available(dacp_client_p dc);
enum dacp_client_playback_state dacp_client_get_playback_state(dacp_client_p dc);
void dacp_client_update_playback_state(dacp_client_p dc);
void dacp_client_next(dacp_client_p dc);
void dacp_client_toggle_playback(dacp_client_p dc);
void dacp_client_previous(dacp_client_p dc);

#endif

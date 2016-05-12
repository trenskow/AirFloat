//
//  dacpclient.h
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
void dacp_client_stop(dacp_client_p dc);
void dacp_client_seek(dacp_client_p dc, float seconds);
void dacp_client_set_volume(dacp_client_p dc, float volume);

#endif

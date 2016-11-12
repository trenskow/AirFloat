//
//  raopsession.h
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

#ifndef _raopsession_h
#define _raopsession_h

#include "settings.h"
#include "dacpclient.h"
#include "raopserver.h"

#ifndef _rsp
typedef struct raop_session_t *raop_session_p;
#define _rsp
#endif

typedef void(*raop_session_client_initiated_callback)(raop_session_p raop_session, void* ctx);
typedef void(*raop_session_client_started_recording_callback)(raop_session_p raop_session, void* ctx);
typedef void(*raop_session_client_ended_recording_callback)(raop_session_p raop_session, void* ctx);
typedef void(*raop_session_client_updated_track_info_callback)(raop_session_p raop_session, const char* title, const char* artist, const char* album, void* ctx);
typedef void(*raop_session_client_updated_track_position_callback)(raop_session_p raop_session, double position, double total, void* ctx);
typedef void(*raop_session_client_updated_artwork_callback)(raop_session_p raop_session, const void* data, size_t data_size, const char* mime_type, void* ctx);
typedef void(*raop_session_client_updated_volume_callback)(raop_session_p raop_session, float volume, void* ctx);
typedef void(*raop_session_ended_callback)(raop_session_p rs, void* ctx);

struct raop_session_t* raop_session_create(raop_server_p server, web_server_connection_p connection, settings_p settings);
void raop_session_destroy(raop_session_p rs);
void raop_session_start(raop_session_p rs);
void raop_session_stop(raop_session_p rs);
void raop_session_set_client_initiated_callback(raop_session_p rs, raop_session_client_initiated_callback callback, void* ctx);
void raop_session_set_client_started_recording_callback(raop_session_p rs, raop_session_client_started_recording_callback callback, void* ctx);
void raop_session_set_client_updated_track_info_callback(raop_session_p rs, raop_session_client_updated_track_info_callback callback, void* ctx);
void raop_session_set_client_updated_track_position_callback(raop_session_p rs, raop_session_client_updated_track_position_callback callback, void* ctx);
void raop_session_set_client_updated_artwork_callback(raop_session_p rs, raop_session_client_updated_artwork_callback callback, void* ctx);
void raop_session_set_client_updated_volume_callback(raop_session_p rs, raop_session_client_updated_volume_callback callback, void* ctx);
void raop_session_set_client_ended_recording_callback(raop_session_p rs, raop_session_client_ended_recording_callback callback, void* ctx);
void raop_session_set_ended_callback(raop_session_p rs, raop_session_ended_callback callback, void* ctx);
bool raop_session_is_recording(raop_session_p rs);
dacp_client_p raop_session_get_dacp_client(raop_session_p rs);

#endif

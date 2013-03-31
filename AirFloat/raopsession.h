//
//  raopsession.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
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
void raop_session_set_client_ended_recording_callback(raop_session_p rs, raop_session_client_ended_recording_callback callback, void* ctx);
void raop_session_set_ended_callback(raop_session_p rs, raop_session_ended_callback callback, void* ctx);
bool raop_session_is_recording(raop_session_p rs);
dacp_client_p raop_session_get_dacp_client(raop_session_p rs);

#endif

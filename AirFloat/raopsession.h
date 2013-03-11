//
//  raopsession.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
//

#ifndef _raopsession_h
#define _raopsession_h

#include "raopserver.h"

struct raop_session_client_updated_progress_info {
    double position;
    double total;
};

struct raop_session_client_updated_artwork_info {
    const void* data;
    size_t data_size;
    const char* mime_type;
};

typedef struct raop_session_t *raop_session_p;

extern const char* raop_session_client_setup_recorder_notification;
extern const char* raop_session_client_recording_started_notification;
extern const char* raop_session_client_updated_progress_notification;
extern const char* raop_session_client_updated_track_notification;
extern const char* raop_session_client_updated_artwork_notification;
extern const char* raop_session_client_torned_down_recorder_notification;

typedef void(*raop_session_ended_callback)(raop_session_p rs, void* ctx);

struct raop_session_t* raop_session_create(raop_server_p server, web_connection_p connection);
void raop_session_destroy(raop_session_p rs);
void raop_session_start(raop_session_p rs);
void raop_session_stop(raop_session_p rs);
void raop_session_set_ended_callback(raop_session_p rs, raop_session_ended_callback callback, void* ctx);
bool raop_session_is_recording(raop_session_p rs);

#endif

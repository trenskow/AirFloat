//
//  raopserver.h
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

#ifndef _raopserver_h
#define _raopserver_h

#include <stdint.h>
#include <stdbool.h>

struct raop_server_settings_t {
    const char* name;
    const char* password;
    bool ignore_source_volume;
};

typedef struct raop_server_t *raop_server_p;

#ifndef _rsp
typedef struct raop_session_t *raop_session_p;
#define _rsp
#endif

typedef void(*raop_server_new_session_callback)(raop_server_p server, raop_session_p new_session, void* ctx);
typedef bool(*raop_server_accept_callback)(raop_server_p server, const char* connection_host, uint16_t connection_port, void* ctx);

raop_server_p raop_server_create(struct raop_server_settings_t settings);
void raop_server_destroy(raop_server_p rs);
bool raop_server_start(raop_server_p rs, uint16_t port);
bool raop_server_is_running(raop_server_p rs);
bool raop_server_is_recording(raop_server_p rs);
struct raop_server_settings_t raop_server_get_settings(raop_server_p rs);
void raop_server_set_settings(raop_server_p rs, struct raop_server_settings_t settings);
void raop_server_stop(raop_server_p rs);
void raop_server_set_new_session_callback(raop_server_p rs, raop_server_new_session_callback new_session_callback, void* ctx);
void raop_server_set_session_accept_callback(raop_server_p rs, raop_server_accept_callback session_accept_callback, void* ctx);

#endif

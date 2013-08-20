//
//  raopserver.c
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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "mutex.h"
#include "zeroconf.h"
#include "settings.h"
#include "webserver.h"
#include "raopsession.h"

#include "obj.h"

#include "raopserver.h"

struct raop_server_t {
    mutex_p mutex;
    settings_p settings;
    web_server_p server;
    zeroconf_raop_ad_p zeroconf_ad;
    bool is_running;
    raop_session_p* sessions;
    uint32_t sessions_count;
    struct {
        raop_server_new_session_callback new_session;
        struct {
            void* new_session;
        } ctx;
    } callbacks;
};

bool _raop_server_web_connection_accept_callback(web_server_p server, web_server_connection_p connection, void* ctx) {
    
    struct raop_server_t* rs = (struct raop_server_t*)ctx;
    
#if (!defined(ALLOW_LOCALHOST))
    if (!sockaddr_equals_host(web_server_connection_get_local_end_point(connection), web_server_connection_get_remote_end_point(connection))) {
#endif
        raop_session_p new_session = raop_session_create(rs, connection, rs->settings);
        
        mutex_lock(rs->mutex);
        
        rs->sessions = (raop_session_p*)realloc(rs->sessions, sizeof(raop_session_p) * (rs->sessions_count + 1));
        rs->sessions[rs->sessions_count] = new_session;
        rs->sessions_count++;
        
        mutex_unlock(rs->mutex);
                
        raop_session_start(new_session);
        
        if (rs->callbacks.new_session != NULL)
            rs->callbacks.new_session(rs, new_session, rs->callbacks.ctx.new_session);
        
        return true;
#if (!defined(ALLOW_LOCALHOST))
    } else
        return false;
#endif
    
}

struct raop_server_t* raop_server_create(struct raop_server_settings_t settings) {
    
    struct raop_server_t* rs = (struct raop_server_t*)obj_create(sizeof(struct raop_server_t));
    
    rs->settings = settings_create(settings.name, settings.password);
    
    rs->mutex = mutex_create_recursive();
    
    rs->server = web_server_create((sockaddr_type)(sockaddr_type_inet_4 | sockaddr_type_inet_6));
    web_server_set_accept_callback(rs->server, _raop_server_web_connection_accept_callback, rs);
    
    return rs;
    
}

void _raop_server_destroy(void* obj) {
    
    struct raop_server_t* rs = (struct raop_server_t*)obj;
    
    web_server_release(rs->server);
    mutex_release(rs->mutex);
    settings_release(rs->settings);
    
}

struct raop_server_t* raop_server_retain(struct raop_server_t* rs) {
    
    return obj_retain(rs);
    
}

struct raop_server_t* raop_server_release(struct raop_server_t* rs) {
    
    return obj_release(rs, _raop_server_destroy);
    
}

bool raop_server_start(struct raop_server_t* rs, uint16_t port) {
    
    if (!rs->is_running) {
        
        bool ret = web_server_start(rs->server, port);
        
        if (ret) {
            rs->is_running = true;
            rs->zeroconf_ad = zeroconf_raop_ad_create(port, settings_get_name(rs->settings));
            log_message(LOG_INFO, "Server started at port %d", port);
        } else
            log_message(LOG_INFO, "Unable to start server at port %d", port);
        
        return ret;
        
    }
    
    return false;
    
}

bool raop_server_is_running(struct raop_server_t* rs) {
    
    mutex_lock(rs->mutex);
    bool ret = rs->is_running;
    mutex_unlock(rs->mutex);
    
    return ret;
    
}

bool raop_server_is_recording(struct raop_server_t* rs) {
    
    bool ret = false;
    
    mutex_lock(rs->mutex);
    uint32_t count = rs->sessions_count;
    mutex_unlock(rs->mutex);
    
    for (uint32_t i = 0 ; i < count ; i++)
        if (raop_session_is_recording(rs->sessions[i])) {
            ret = true;
            break;
        }
    
    return ret;
    
}

struct raop_server_settings_t raop_server_get_settings(struct raop_server_t* rs) {
    
    return  (struct raop_server_settings_t){ settings_get_name(rs->settings), settings_get_password(rs->settings) };
    
}

void raop_server_set_settings(struct raop_server_t* rs, struct raop_server_settings_t settings) {
    
    const char* old_name = settings_get_name(rs->settings);
    char* old_name_c = (char*)malloc(strlen(settings_get_name(rs->settings) + 1));
    strcpy(old_name_c, old_name);
    
    settings_set_name(rs->settings, settings.name);
    settings_set_password(rs->settings, settings.password);
    
    const char* new_name = settings_get_name(rs->settings);
    
    if (strcmp(old_name_c, new_name) != 0) {
        zeroconf_raop_ad_release(rs->zeroconf_ad);
        rs->zeroconf_ad = zeroconf_raop_ad_create(sockaddr_get_port(web_server_get_local_end_point(rs->server, sockaddr_type_inet_4)), new_name);
    }
    
    free(old_name_c);
    
}

void raop_server_stop(struct raop_server_t* rs) {
    
    mutex_lock(rs->mutex);
    
    if (rs->is_running) {
        
        rs->is_running = false;
        
        for (uint32_t i = 0 ; i < rs->sessions_count ; i++)
            raop_session_release(rs->sessions[i]);
        
        free(rs->sessions);
        
        rs->sessions = NULL;
        rs->sessions_count = 0;
        
        rs->zeroconf_ad = zeroconf_raop_ad_release(rs->zeroconf_ad);
        
        web_server_stop(rs->server);
        
    }
    
    mutex_unlock(rs->mutex);
    
}

void raop_server_set_new_session_callback(struct raop_server_t* rs, raop_server_new_session_callback new_session_callback, void* ctx) {
    
    rs->callbacks.new_session = new_session_callback;
    rs->callbacks.ctx.new_session = ctx;
    
}

void raop_server_session_ended(struct raop_server_t* rs, raop_session_p session) {
    
    mutex_lock(rs->mutex);
    
    for (uint32_t i = 0 ; i < rs->sessions_count ; i++)
        if (rs->sessions[i] == session) {
            raop_session_release(rs->sessions[i]);
            for (uint32_t a = i + 1 ; a < rs->sessions_count ; a++)
                rs->sessions[a - 1] = rs->sessions[a];
            rs->sessions_count--;
            break;
        }
    
    mutex_unlock(rs->mutex);
    
}

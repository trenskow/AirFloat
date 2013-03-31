//
//  raopserver.c
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
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
#include "raopserver.h"

struct raop_server_t {
    mutex_p mutex;
    settings_p settings;
    web_server_p server;
    zeroconf_raop_ad_p zeroconf_ad;
    bool is_running;
    raop_session_p* sessions;
    uint32_t sessions_count;
    raop_server_new_session_callback new_session_callback;
    void* new_session_ctx;
};

void _raop_server_session_ended_callback(raop_session_p session, void* ctx) {
    
    struct raop_server_t* rs = (struct raop_server_t*)ctx;
    
    mutex_lock(rs->mutex);
    
    for (uint32_t i = 0 ; i < rs->sessions_count ; i++)
        if (rs->sessions[i] == session) {
            raop_session_destroy(rs->sessions[i]);
            for (uint32_t a = i + 1 ; a < rs->sessions_count ; a++)
                rs->sessions[a - 1] = rs->sessions[a];
            rs->sessions_count--;
            break;
        }
    
    mutex_unlock(rs->mutex);
    
}

bool _raop_server_web_connection_accept_callback(web_server_p server, web_server_connection_p connection, void* ctx) {
    
    struct raop_server_t* rs = (struct raop_server_t*)ctx;
    
#if (!defined(ALLOW_LOCALHOST))
    if (!sockaddr_equals_host(web_connection_get_local_end_point(connection), web_connection_get_remote_end_point(connection))) {
#endif
        raop_session_p new_session = raop_session_create(rs, connection, rs->settings);
        
        mutex_lock(rs->mutex);
        
        rs->sessions = (raop_session_p*)realloc(rs->sessions, sizeof(raop_session_p) * (rs->sessions_count + 1));
        rs->sessions[rs->sessions_count] = new_session;
        rs->sessions_count++;
        
        mutex_unlock(rs->mutex);
        
        raop_session_set_ended_callback(new_session, _raop_server_session_ended_callback, rs);
        
        raop_session_start(new_session);
        
        if (rs->new_session_callback != NULL)
            rs->new_session_callback(rs, new_session, rs->new_session_ctx);
        
        return true;
#if (!defined(ALLOW_LOCALHOST))
    } else
        return false;
#endif
    
}

struct raop_server_t* raop_server_create(struct raop_server_settings_t settings) {
    
    struct raop_server_t* rs = (struct raop_server_t*)malloc(sizeof(struct raop_server_t));
    bzero(rs, sizeof(struct raop_server_t));
    
    rs->settings = settings_create(settings.name, settings.password);
    
    rs->mutex = mutex_create();
    
    rs->server = web_server_create((sockaddr_type)(sockaddr_type_inet_4 | sockaddr_type_inet_6));
    web_server_set_accept_callback(rs->server, _raop_server_web_connection_accept_callback, rs);
    
    return rs;
    
}

void raop_server_destroy(struct raop_server_t* rs) {
    
    web_server_destroy(rs->server);
    
    mutex_destroy(rs->mutex);
    
    free(rs);
    
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
        zeroconf_raop_ad_destroy(rs->zeroconf_ad);
        rs->zeroconf_ad = zeroconf_raop_ad_create(sockaddr_get_port(web_server_get_local_end_point(rs->server, sockaddr_type_inet_4)), new_name);
    }
    
    free(old_name_c);
    
}

void raop_server_stop(struct raop_server_t* rs) {
    
    mutex_lock(rs->mutex);
    
    if (rs->is_running) {
        
        rs->is_running = false;
        
        while (rs->sessions_count > 0) {
            mutex_unlock(rs->mutex);
            raop_session_destroy(rs->sessions[0]);
            mutex_lock(rs->mutex);
        }
        
        free(rs->sessions);
        
        rs->sessions = NULL;
        rs->sessions_count = 0;
        
        zeroconf_raop_ad_destroy(rs->zeroconf_ad);
        
        web_server_stop(rs->server);
        
    }
    
    mutex_unlock(rs->mutex);
    
}

void raop_server_set_new_session_callback(struct raop_server_t* rs, raop_server_new_session_callback new_session_callback, void* ctx) {
    
    rs->new_session_callback = new_session_callback;
    rs->new_session_ctx = ctx;
    
}

void raop_server_session_ended(struct raop_server_t* rs, raop_session_p session) {
    
    mutex_lock(rs->mutex);
    
    for (uint32_t i = 0 ; i < rs->sessions_count ; i++)
        if (rs->sessions[i] == session) {
            raop_session_destroy(session);
            for (uint32_t x = i ; x < rs->sessions_count - 1 ; x++)
                rs->sessions[x] = rs->sessions[x + 1];
            break;
        }
    
    mutex_unlock(rs->mutex);
    
}

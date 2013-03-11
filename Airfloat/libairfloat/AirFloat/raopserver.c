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
#include "bonjour.h"
#include "raopserver.h"
#include "webserver.h"
#include "notificationcenter.h"
#include "raopsession.h"

struct raop_server_t {
    mutex_p mutex;
    web_server_p server;
    bonjour_ad_p bonjour_ad;
    bool is_running;
    raop_session_p* sessions;
    uint32_t sessions_count;
};

const char* raop_server_localhost_refused_notification = "raop_server_localhost_refused_notification";

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

bool _raop_server_web_connection_accept_callback(web_server_p server, web_connection_p connection, void* ctx) {
    
    struct raop_server_t* rs = (struct raop_server_t*)ctx;
    
#ifndef DEBUG
    if (!sockaddr_equals_host(web_connection_get_local_end_point(connection), web_connection_get_remote_end_point(connection))) {
#endif
        raop_session_p new_session = raop_session_create(rs, connection);
        
        mutex_lock(rs->mutex);
        
        rs->sessions = (raop_session_p*)realloc(rs->sessions, sizeof(raop_session_p) * (rs->sessions_count + 1));
        rs->sessions[rs->sessions_count] = new_session;
        rs->sessions_count++;
        
        mutex_unlock(rs->mutex);
        
        raop_session_set_ended_callback(new_session, _raop_server_session_ended_callback, rs);
        
        raop_session_start(new_session);
        
        return true;
#ifndef DEBUG
    } else {
        notification_center_post_notification(raop_server_localhost_refused_notification, rs, NULL);
        return false;
    }
#endif
    
}

struct raop_server_t* raop_server_create() {
    
    struct raop_server_t* rs = (struct raop_server_t*)malloc(sizeof(struct raop_server_t));
    bzero(rs, sizeof(struct raop_server_t));
    
    rs->mutex = mutex_create();
    
    rs->server = web_server_create((sockaddr_type)(sockaddr_type_inet_4 | sockaddr_type_inet_6));
    web_server_set_accept_callback(rs->server, _raop_server_web_connection_accept_callback, rs);
    
    return rs;
    
}

void raop_server_destroy(struct raop_server_t* rs) {
    
    web_server_stop(rs->server);
    web_server_destroy(rs->server);
    
    mutex_destroy(rs->mutex);
    
    free(rs);
    
}

bool raop_server_start(struct raop_server_t* rs, uint16_t port) {
    
    if (!rs->is_running) {
        
        bool ret = web_server_start(rs->server, port);
        
        if (ret) {
            rs->is_running = true;
            rs->bonjour_ad = bonjour_ad_create(port);
            log_message(LOG_INFO, "Server started at port %d", port);
        } else
            log_message(LOG_INFO, "Unable to start server at port %d", port);
        
        return ret;
        
    }
    
    log_message(LOG_ERROR, "Cannot start server: already running");
    
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
    
    for (uint32_t i = 0 ; i < rs->sessions_count ; i++)
        if (raop_session_is_recording(rs->sessions[i])) {
            ret = true;
            break;
        }
    
    mutex_unlock(rs->mutex);
    
    return ret;
    
}

void raop_server_stop(struct raop_server_t* rs) {
    
    mutex_lock(rs->mutex);
    
    if (rs->is_running) {
        
        for (uint32_t i = 0 ; i < rs->sessions_count ; i++)
            raop_session_destroy(rs->sessions[i]);
        
        rs->sessions_count = 0;
        rs->is_running = false;
        
        web_server_stop(rs->server);
        
    } else
        log_message(LOG_ERROR, "Cannot stop server: already stopped");
    
    mutex_unlock(rs->mutex);
    
}

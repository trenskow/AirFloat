//
//  dacpclient.c
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
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "zeroconf.h"
#include "dmap.h"
#include "sockaddr.h"
#include "webrequest.h"
#include "webresponse.h"
#include "webclientconnection.h"

#include "dacpclient.h"

struct dacp_client_t {
    struct sockaddr* end_point;
    char* identifier;
    char* active_remove;
    zeroconf_dacp_discover_p dacp_discover;
    web_client_connection_p web_connection;
    struct {
        dacp_client_controls_became_available_callback controls_became_available;
        dacp_client_controls_became_unavailable_callback controls_became_unavailable;
        dacp_client_playback_state_changed_callback playback_state_changed;
        struct {
            void* controls_became_available;
            void* controls_became_unavailable;
            void* playback_state_changed;
        } ctx;
    } callbacks;
    enum dacp_client_playback_state playback_state;
    bool is_destroyed;
};

void _dacp_client_web_connection_connected_callback(web_client_connection_p connection, void* ctx) {
    
    struct dacp_client_t* dc = (struct dacp_client_t*)ctx;
    
    log_message(LOG_INFO, "Connected!");
    
    if (dc->callbacks.controls_became_available != NULL)
        dc->callbacks.controls_became_available(dc, dc->callbacks.ctx.controls_became_available);
    
}

void _dacp_client_web_connection_connect_failed_callback(web_client_connection_p connection, void* ctx) {
    
    struct dacp_client_t* dc = (struct dacp_client_t*)ctx;
    
    web_client_connection_destroy(dc->web_connection);
    dc->web_connection = NULL;
    
}

void _dacp_client_web_connection_disconnected_callback(web_client_connection_p connection, void* ctx) {
    
    struct dacp_client_t* dc = (struct dacp_client_t*)ctx;
    
    if (dc->web_connection != NULL) {
        
        if (dc->callbacks.controls_became_unavailable != NULL)
            dc->callbacks.controls_became_unavailable(dc, dc->callbacks.ctx.controls_became_unavailable);
        
        if (!dc->is_destroyed) {
            web_client_connection_destroy(dc->web_connection);
            dc->web_connection = NULL;
        }
        
    }
    
}

void _dacp_client_web_connection_response_received_callback(web_client_connection_p connection, web_request_p request, web_response_p response, void* ctx) {
    
    struct dacp_client_t* dc = (struct dacp_client_t*)ctx;
    
    if (web_response_get_status(response) == 200 && dc->callbacks.playback_state_changed != NULL) {
        
        web_headers_p headers = web_response_get_headers(response);
        
        size_t content_length;
        const char* s_content_type = web_headers_value(headers, "Content-Type");
        
        if (s_content_type != NULL && strcmp(s_content_type, "application/x-dmap-tagged") == 0 && (content_length = web_response_get_content(response, NULL, 0)) > 0) {
            
            char data[content_length];
            
            web_response_get_content(response, data, content_length);
            
            dmap_p dmap = dmap_create();
            dmap_parse(dmap, data, content_length);
            
            dmap_p container = dmap_container_for_atom_identifer(dmap, "com.airfloat.nowplayingcontainer");
            
            if (container != NULL) {
                
                char now_playing = dmap_char_for_atom_identifer(container, "com.airfloat.nowplayingstatus");
                dc->playback_state = now_playing;
                dc->callbacks.playback_state_changed(dc, dc->playback_state, dc->callbacks.ctx.playback_state_changed);
                
            }
            
            dmap_destroy(dmap);
            
        }
        
    }
    
}

void _dacp_client_zeroconf_resolved_callback(zeroconf_dacp_discover_p zeroconf_dacp_discover, const char* name, struct sockaddr** end_points, uint32_t end_points_count, void* ctx) {
    
    struct dacp_client_t* dc = (struct dacp_client_t*)ctx;
    
    if (dc->web_connection == NULL && strlen(name) > 12 && memcmp(name, "iTunes_Ctrl_", 12) == 0 && strcmp(name + 12, dc->identifier) == 0) {
        
        for (uint32_t i = 0 ; i < end_points_count ; i++)
            if (sockaddr_equals_host(end_points[i], dc->end_point)) {
                
                dc->web_connection = web_client_connection_create();
                
                web_client_connection_set_connected_callback(dc->web_connection, _dacp_client_web_connection_connected_callback, dc);
                web_client_connection_set_connect_failed_callback(dc->web_connection, _dacp_client_web_connection_connect_failed_callback, dc);
                web_client_connection_set_disconneced_callback(dc->web_connection, _dacp_client_web_connection_disconnected_callback, dc);
                web_client_connection_set_response_received_callback(dc->web_connection, _dacp_client_web_connection_response_received_callback, dc);
                web_client_connection_connect(dc->web_connection, end_points[i]);
                
                break;
                
            }
                
    }
    
}

void _dacp_client_send_request(struct dacp_client_t* dc, const char* request_name) {
    
    if (dacp_client_is_available(dc)) {
        
        web_request_p request = web_request_create();
        
        char path[strlen(request_name) + 13];
        sprintf(path, "/ctrl-int/1/%s", request_name);
        
        web_request_set_method(request, "GET");
        web_request_set_path(request, path);
        web_request_set_protocol(request, "HTTP/1.1");
        
        web_headers_set_value(web_request_get_headers(request), "Active-Remote", dc->active_remove);
        
        web_client_connection_send_request(dc->web_connection, request);
        
        web_request_destroy(request);
        
    }
    
}

struct dacp_client_t* dacp_client_create(struct sockaddr* end_point, const char* identifier, const char* active_remote) {
    
    struct dacp_client_t* dc = (struct dacp_client_t*)malloc(sizeof(struct dacp_client_t));
    bzero(dc, sizeof(struct dacp_client_t));
    
    dc->end_point = sockaddr_copy(end_point);
    sockaddr_set_port(dc->end_point, 3689);
    
    dc->identifier = malloc(strlen(identifier) + 1);
    strcpy(dc->identifier, identifier);
    dc->active_remove = malloc(strlen(active_remote) + 1);
    strcpy(dc->active_remove, active_remote);
        
    dc->dacp_discover = zeroconf_dacp_discover_create();
    zeroconf_dacp_discover_set_callback(dc->dacp_discover, _dacp_client_zeroconf_resolved_callback, dc);
    
    dc->playback_state = dacp_client_playback_state_stopped;
    
    return dc;
    
}

void dacp_client_destroy(struct dacp_client_t* dc) {
    
    sockaddr_destroy(dc->end_point);
    
    dc->is_destroyed = true;
    
    if (dc->dacp_discover != NULL) {
        zeroconf_dacp_discover_destroy(dc->dacp_discover);
        dc->dacp_discover = NULL;
    }
    
    if (dc->web_connection != NULL) {
        web_client_connection_destroy(dc->web_connection);
        dc->web_connection = NULL;
    }
    
    free(dc->identifier);
    free(dc->active_remove);
    
    free(dc);
    
}

void dacp_client_set_controls_became_available_callback(struct dacp_client_t* dc, dacp_client_controls_became_available_callback callback, void* ctx) {
    
    dc->callbacks.controls_became_available = callback;
    dc->callbacks.ctx.controls_became_available = ctx;
    
}

void dacp_client_set_controls_became_unavailable_callback(struct dacp_client_t* dc, dacp_client_controls_became_unavailable_callback callback, void* ctx) {
    
    dc->callbacks.controls_became_unavailable = callback;
    dc->callbacks.ctx.controls_became_unavailable = ctx;
    
}

void dacp_client_set_playback_state_changed_callback(struct dacp_client_t* dc, dacp_client_playback_state_changed_callback callback, void* ctx) {
    
    dc->callbacks.playback_state_changed = callback;
    dc->callbacks.ctx.playback_state_changed = ctx;
    
}

bool dacp_client_is_available(struct dacp_client_t* dc) {
    
    return (dc->web_connection != NULL && web_client_connection_is_connected(dc->web_connection));
    
}

enum dacp_client_playback_state dacp_client_get_playback_state(dacp_client_p dc) {
    
    return dc->playback_state;
    
}

void dacp_client_update_playback_state(struct dacp_client_t* dc) {
    
    _dacp_client_send_request(dc, "playstatusupdate");
    
}

void dacp_client_next(struct dacp_client_t* dc) {
    
    _dacp_client_send_request(dc, "nextitem");
    
}

void dacp_client_toggle_playback(struct dacp_client_t* dc) {
    
    _dacp_client_send_request(dc, "playpause");
    
}

void dacp_client_previous(struct dacp_client_t* dc) {
    
    _dacp_client_send_request(dc, "previtem");
    
}

void dacp_client_stop(struct dacp_client_t* dc) {
    
    _dacp_client_send_request(dc, "stop");
    
}

void dacp_client_seek(dacp_client_p dc, float seconds) {
    
    uint32_t milliseconds = seconds * 1000;
    char cmd[100];
    sprintf(cmd, "setproperty?dacp.playingtime=%d", milliseconds);
    _dacp_client_send_request(dc, cmd);
    
}

void dacp_client_set_volume(dacp_client_p dc, float volume) {
    
    if (volume >= 0 && volume <= 100.0) {
        char cmd[100];
        sprintf(cmd, "setproperty?dmcp.volume=%f", volume);
        _dacp_client_send_request(dc, cmd);
    }
    
}

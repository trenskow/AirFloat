//
//  raopsession.c
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
#include <ctype.h>
#include <assert.h>
#include <math.h>

#include <netinet/in.h>

#include "log.h"
#include "mutex.h"
#include "base64.h"
#include "hex.h"
#include "settings.h"
#include "hardware.h"

#include "settings.h"

#include "parameters.h"
#include "dmap.h"

#include "webtools.h"
#include "webserverconnection.h"

#include "dacpclient.h"

#include "crypt.h"
#include "decoder.h"

#include "audioqueue.h"
#include "audiooutput.h"
#include "raopserver.h"
#include "rtprecorder.h"

#include "raopsession.h"

#define MAX(x,y) (x > y ? x : y)

void raop_server_session_ended(raop_server_p rs, struct raop_session_t* session);

struct raop_rtp_session_t {
    audio_queue_p queue;
    rtp_recorder_p recorder;
    uint32_t session_id;
};

struct raop_session_t {
    object_p object;
    mutex_p mutex;
    bool is_running;
    raop_server_p server;
    char* password;
    bool ignore_source_volume;
    web_server_connection_p raop_connection;
    char authentication_digest_nonce[33];
    dacp_client_p dacp_client;
    char* user_agent;
    crypt_aes_p crypt_aes;
    decoder_p decoder;
    struct raop_rtp_session_t* rtp_session;
    uint32_t rtp_last_session_id;
    struct {
        raop_session_client_initiated_callback initiated;
        raop_session_client_started_recording_callback started_recording;
        raop_session_client_updated_track_info_callback updated_track_info;
        raop_session_client_updated_track_position_callback updated_track_position;
        raop_session_client_updated_artwork_callback updated_artwork;
        raop_session_client_updated_volume_callback updated_volume;
        raop_session_client_ended_recording_callback ended_recording;
        raop_session_ended_callback ended;
        struct {
            void* initiated;
            void* started_recording;
            void* updated_track_info;
            void* updated_track_position;
            void* updated_artwork;
            void* updated_volume;
            void* ended_recording;
            void* ended;
        } ctx;
    } callbacks;
    
    unsigned int start_rtp_timestamp;
    double total_length;
};

void recorder_updated_track_position_callback(rtp_recorder_p rr, unsigned int curr, void* ctx) {
    struct raop_session_t* rs = (struct raop_session_t*)ctx;
    struct decoder_output_format_t output_format = decoder_get_output_format(rs->decoder);
    
    double srate = (double)output_format.sample_rate;
    if (srate == 0.0) {
        srate = 44100;
    }
    double position = (double)(curr - rs->start_rtp_timestamp) / srate;
    if (rs->callbacks.updated_track_position != NULL && (rs->total_length == 0 || position < rs->total_length)) {
        rs->callbacks.updated_track_position(rs, position, rs->total_length, rs->callbacks.ctx.updated_track_position);
    }
}

bool _raop_session_check_authentication(struct raop_session_t* rs, const char* method, const char* uri, const char* authentication_parameter) {
    
    assert(method != NULL && uri != NULL);
    
    bool ret = (rs->password == NULL);
    
    if (ret == false) {
        
        if (authentication_parameter != NULL) {
            
            const char* param_begin = strstr(authentication_parameter, " ") + 1;
            if (param_begin) {
                
                parameters_p parameters = parameters_create(param_begin, strlen(param_begin), parameters_type_http_authentication);
                
                const char* nonce = parameters_value_for_key(parameters, "nonce");
                const char* response = parameters_value_for_key(parameters, "response");
                
                char w_response[strlen(response) + 1];
                strcpy(w_response, response);
                
                // Check if nonce is correct
                if (nonce != NULL && strlen(nonce) == 32 && strcmp(nonce, rs->authentication_digest_nonce) == 0) {
                    
                    const char* username = parameters_value_for_key(parameters, "username");
                    const char* realm =  parameters_value_for_key(parameters, "realm");
                    size_t pw_len = strlen(rs->password);
                    
                    char a1pre[strlen(username) + strlen(realm) + pw_len + 3];
                    sprintf(a1pre, "%s:%s:%s", username, realm, rs->password);
                    
                    char a2pre[strlen(method) + strlen(uri) + 2];
                    sprintf(a2pre, "%s:%s", method, uri);
                    
                    uint16_t a1[16], a2[16];
                    crypt_md5_hash(a1pre, strlen(a1pre), a1, 16);
                    crypt_md5_hash(a2pre, strlen(a2pre), a2, 16);
                    
                    char ha1[33], ha2[33];
                    ha1[32] = ha2[32] = '\0';
                    hex_encode(a1, 16, ha1, 32);
                    hex_encode(a2, 16, ha2, 32);
                    
                    char finalpre[67 + strlen(rs->authentication_digest_nonce)];
                    sprintf(finalpre, "%s:%s:%s", ha1, rs->authentication_digest_nonce, ha2);
                    
                    uint16_t final[16];
                    crypt_md5_hash(finalpre, strlen(finalpre), final, 16);
                    
                    char hfinal[33];
                    hfinal[32] = '\0';
                    hex_encode(final, 16, hfinal, 32);
                    
                    for (int i = 0 ; i < 32 ; i++) {
                        hfinal[i] = tolower(hfinal[i]);
                        w_response[i] = tolower(w_response[i]);
                    }
                    
                    if (strcmp(hfinal, w_response) == 0)
                        ret = true;
                    else
                        log_message(LOG_INFO, "Authentication failure");
                    
                }
                
                object_release(parameters);
                
            }
            
        } else
            log_message(LOG_INFO, "Authentication header missing");
        
    }
    
    return ret;
    
}

void _raop_session_get_apple_response(struct raop_session_t* rs, const char* challenge, size_t challenge_length, char* response, size_t* response_length) {
    
    char decoded_challenge[1000];
    size_t actual_length = base64_decode(challenge, decoded_challenge);
    
    if (actual_length != 16) {
        log_message(LOG_ERROR, "Apple-Challenge: Expected 16 bytes - got %d", actual_length);
    }
    
    endpoint_p local_endpoint = web_server_connection_get_local_endpoint(rs->raop_connection);
    uint64_t hw_identifier = hardware_identifier();
    
    size_t response_size = 32;
    char a_response[48]; // IPv6 responds with 48 bytes
    
    memset(a_response, 0, sizeof(a_response));
    
    struct sockaddr* addr = (struct sockaddr*)endpoint_get_sockaddr(local_endpoint);
    
    if (endpoint_is_ipv6(local_endpoint)) {
        
        response_size = 48;
        
        memcpy(a_response, decoded_challenge, actual_length);
        memcpy(&a_response[actual_length], &((struct sockaddr_in6*)addr)->sin6_addr, 16);
        memcpy(&a_response[actual_length + 16], &((char*)&hw_identifier)[2], 6);
        
    } else {
        
        memcpy(a_response, decoded_challenge, actual_length);
        memcpy(&a_response[actual_length], &((struct sockaddr_in*)addr)->sin_addr.s_addr, 4);
        memcpy(&a_response[actual_length + 4], &((char*)&hw_identifier)[2], 6);
        
    }
    
    unsigned char clear_response[256];
    memset(clear_response, 0xFF, 256);
    clear_response[0] = 0;
    clear_response[1] = 1;
    clear_response[256 - (response_size + 1)] = 0;
    memcpy(&clear_response[256 - response_size], a_response, response_size);
    
    unsigned char encrypted_response[256];
    size_t size = crypt_apple_private_encrypt(clear_response, 256, encrypted_response, 256);
    
    if (size > 0) {
        
        char* a_encrypted_response;
        size_t a_len = base64_encode(encrypted_response, size, &a_encrypted_response);
        
        if (response != NULL)
            memcpy(response, a_encrypted_response, a_len);
        if (response_length != NULL)
            *response_length = a_len;
        
        free(a_encrypted_response);
        
    } else {
        log_message(LOG_ERROR, "Unable to encrypt Apple response");
        if (response_length != NULL)
            *response_length = 0;
    }
    
}

void _raop_session_audio_queue_received_audio_callback(audio_queue_p aq, void* ctx) {
    
    struct raop_session_t* rs = (struct raop_session_t*)ctx;
    
    if (rs->dacp_client != NULL)
        dacp_client_update_playback_state(rs->dacp_client);
    
}

void _raop_session_raop_connection_request_callback(web_server_connection_p connection, web_request_p request, void* ctx) {
    
    struct raop_session_t* rs = (struct raop_session_t*)ctx;
    
    int c_seq = 0;
    
    bool keep_alive = true;
    
    const char* cmd = web_request_get_method(request);
    const char* path = web_request_get_path(request);
    web_headers_p request_headers = web_request_get_headers(request);
    
    web_response_p response = web_response_create();
    web_headers_p response_headers = web_response_get_headers(response);
    
    web_response_set_status(response, 200, "OK");
    
    if (cmd != NULL && path != NULL) {
        
        parameters_p parameters = NULL;
        
        size_t content_length;
        if ((content_length = web_request_get_content(request, NULL, 0)) > 0) {
            
            const char* content_type = web_headers_value(request_headers, "Content-Type");
            
            if (strcmp(content_type, "application/sdp") == 0 || strcmp(content_type, "text/parameters") == 0) {
                
                char* content[content_length];
                
                web_request_get_content(request, content, content_length);
                
                content_length = web_tools_convert_new_lines(content, content_length);
                
                if (strcmp(content_type, "application/sdp") == 0)
                    parameters = parameters_create(content, content_length, parameters_type_sdp);
                else if (strcmp(content_type, "text/parameters") == 0)
                    parameters = parameters_create(content, content_length, parameters_type_text);
                
            }
            
        }
        
        mutex_lock(rs->mutex);
        
        const char *user_agent;
        
        if (rs->user_agent == NULL && (user_agent = web_headers_value(request_headers, "User-Agent")) != NULL) {
            rs->user_agent = (char*)malloc(strlen(user_agent) + 1);
            strcpy(rs->user_agent, user_agent);
        }
        
        struct raop_rtp_session_t* rtp_session = rs->rtp_session;
        
        mutex_unlock(rs->mutex);
        
        web_headers_set_value(response_headers, "Server", "AirTunes/105.1");
        web_headers_set_value(response_headers, "CSeq", "%d", c_seq);
        
        if (_raop_session_check_authentication(rs, cmd, path, web_headers_value(request_headers, "Authorization"))) {
            
            if (0 == strcmp(cmd, "OPTIONS"))
                web_headers_set_value(response_headers, "Public", "ANNOUNCE, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, GET_PARAMETER, SET_PARAMETER, POST, GET");
            else if (0 == strcmp(cmd, "ANNOUNCE")) {
                
                mutex_lock(rs->mutex);
                if (rs->dacp_client == NULL) {
                    
                    const char* dacp_id;
                    const char* active_remote;
                    if ((dacp_id = web_headers_value(request_headers, "DACP-ID")) != NULL && (active_remote = web_headers_value(request_headers, "Active-Remote")) != NULL)
                        rs->dacp_client = dacp_client_create(web_server_connection_get_remote_endpoint(rs->raop_connection), dacp_id, active_remote);
                    
                }
                mutex_unlock(rs->mutex);
                
                const char* codec = NULL;
                uint32_t codec_identifier;
                
                const char* rtpmap;
                if ((rtpmap = parameters_value_for_key(parameters, "a-rtpmap")) != NULL) {
                    
                    codec_identifier = atoi(rtpmap);
                    for (uint32_t i = 0 ; i < strlen(rtpmap) - 1 ; i++)
                        if (rtpmap[i] == ' ') {
                            codec = &rtpmap[i+1];
                            break;
                        }
                    
                    const char* fmtp;
                    if ((fmtp = parameters_value_for_key(parameters, "a-fmtp")) != NULL && atoi(fmtp) == codec_identifier) {
                        
                        char* rtp_fmtp = (char*)fmtp;
                        while (rtp_fmtp[0] != ' ')
                            rtp_fmtp++;
                        rtp_fmtp++;
                        
                        mutex_lock(rs->mutex);
                        rs->decoder = decoder_create(codec, rtp_fmtp);
                        mutex_unlock(rs->mutex);
                        
                        const char* aes_key_base64_encrypted;
                        if ((aes_key_base64_encrypted = parameters_value_for_key(parameters, "a-rsaaeskey")) != NULL) {
                            
                            size_t aes_key_base64_encrypted_padded_length = strlen(aes_key_base64_encrypted) + 5;
                            char aes_key_base64_encrypted_padded[aes_key_base64_encrypted_padded_length];
                            size_t size = base64_pad(aes_key_base64_encrypted, strlen(aes_key_base64_encrypted), aes_key_base64_encrypted_padded, aes_key_base64_encrypted_padded_length);
                            char aes_key_encryptet[size];
                            size = base64_decode(aes_key_base64_encrypted_padded, aes_key_encryptet);
                            
                            unsigned char aes_key[size + 1];
                            size = crypt_apple_private_decrypt(aes_key_encryptet, size, aes_key, size);
                            
                            log_message(LOG_INFO, "AES key length: %d bits", size * 8);
                            
                            const char* aes_initializer_base64 = parameters_value_for_key(parameters, "a-aesiv");
                            size_t aes_initializer_base64_encoded_padded_length = strlen(aes_initializer_base64) + 5;
                            char aes_initializer_base64_encoded_padded[aes_initializer_base64_encoded_padded_length];
                            size = base64_pad(aes_initializer_base64, strlen(aes_initializer_base64), aes_initializer_base64_encoded_padded, aes_initializer_base64_encoded_padded_length);
                            char aes_initializer[size];
                            size = base64_decode(aes_initializer_base64_encoded_padded, aes_initializer);
                            
                            mutex_lock(rs->mutex);
                            if (rs->crypt_aes != NULL) {
                                object_release(rs->crypt_aes);
                                rs->crypt_aes = NULL;
                            }
                            rs->crypt_aes = crypt_aes_create(aes_key, aes_initializer, size);
                            mutex_unlock(rs->mutex);
                            
                        }
                        
                    } else
                        web_response_set_status(response, 400, "Bad Request");
                    
                } else
                    web_response_set_status(response, 400, "Bad Request");
                
            } else if (0 == strcmp(cmd, "SETUP")) {
                
                bool is_recording = raop_server_is_recording(rs->server);
                
                if (!is_recording) {
                    
                    const char* transport;
                    if ((transport = web_headers_value(request_headers, "Transport"))) {
                        
                        parameters_p transport_params = parameters_create(transport, strlen(transport) + 1, parameters_type_http_header);
                        
                        const char* s_control_port;
                        const char* s_timing_port;
                        uint16_t control_port = 0, timing_port = 0;
                        
                        if ((s_control_port = parameters_value_for_key(transport_params, "control_port")) != NULL)
                            control_port = atoi(s_control_port);
                        if ((s_timing_port = parameters_value_for_key(transport_params, "timing_port")) != NULL)
                            timing_port = atoi(s_timing_port);
                        
                        mutex_lock(rs->mutex);
                        
                        audio_queue_p audio_queue = audio_queue_create(rs->decoder);
                        
                        audio_queue_set_received_audio_callback(audio_queue, _raop_session_audio_queue_received_audio_callback, rs);
                        
                        endpoint_p local_endpoint = web_server_connection_get_local_endpoint(rs->raop_connection);
                        endpoint_p remote_endpoint = web_server_connection_get_remote_endpoint(rs->raop_connection);
                        rtp_recorder_p new_recorder = rtp_recorder_create(rs->crypt_aes, audio_queue, local_endpoint, remote_endpoint, control_port, timing_port);
                        rtp_recorder_set_updated_track_position_callback(new_recorder, recorder_updated_track_position_callback, rs);
                        
                        uint32_t session_id = ++rs->rtp_last_session_id;
                        rs->rtp_session = (struct raop_rtp_session_t*)malloc(sizeof(struct raop_rtp_session_t));
                        rs->rtp_session->queue = audio_queue;
                        rs->rtp_session->recorder = new_recorder;
                        rs->rtp_session->session_id = session_id;
                        
                        web_headers_set_value(response_headers, "Session", "%d", session_id);
                        
                        parameters_set_value(transport_params, "timing_port", "%d", rtp_recorder_get_timing_port(rs->rtp_session->recorder));
                        parameters_set_value(transport_params, "control_port", "%d", rtp_recorder_get_control_port(rs->rtp_session->recorder));
                        parameters_set_value(transport_params, "server_port", "%d", rtp_recorder_get_streaming_port(rs->rtp_session->recorder));
                        
                        size_t transport_reply_len = parameters_write(transport_params, NULL, 0, parameters_type_http_header);
                        char transport_reply[transport_reply_len];
                        parameters_write(transport_params, transport_reply, transport_reply_len, parameters_type_http_header);
                        
                        web_headers_set_value(response_headers, "Transport", transport_reply);
                        
                        if (parameters != NULL) {
                            object_release(parameters);
                            parameters = NULL;
                        }
                        
                        object_release(transport_params);
                        
                        mutex_unlock(rs->mutex);
                        
                        if (rs->callbacks.initiated != NULL)
                            rs->callbacks.initiated(rs, rs->callbacks.ctx.initiated);
                        
                    } else
                        web_response_set_status(response, 400, "Bad Request");
                    
                } else
                    web_response_set_status(response, 453, "Not Enough Bandwidth");
                
            } else if (0 == strcmp(cmd, "RECORD") && rtp_session != NULL) {
                
                if (rtp_recorder_start(rtp_session->recorder)) {
                    
                    audio_queue_start(rtp_session->queue);
                    
                    web_headers_set_value(response_headers, "Audio-Latency", "11025");
                    
                    if (rs->callbacks.started_recording != NULL)
                        rs->callbacks.started_recording(rs, rs->callbacks.ctx.started_recording);
                    
                } else
                    web_response_set_status(response, 400, "Bad Request");
                
            } else if (0 == strcmp(cmd, "SET_PARAMETER") && rtp_session != NULL) {
                
                if (parameters != NULL) {
                    
                    const char* volume;
                    const char* progress;
                    if ((volume = parameters_value_for_key(parameters, "volume"))) {
                        
                        if (!rs->ignore_source_volume) {
                            
                            //The volume is a float value representing the audio attenuation in dB
                            //A value of –144 means the audio is muted. Then it goes from –30 to 0.
                            float volume_db;
                            sscanf(volume, "%f", &volume_db);
                            
                            // input       : output
                            // -144.0      : silence
                            // -30.0 - 0.0 : 0.0 - 1.0
                            
                            float volume_p = 0;
                            if (volume_db < -144) {
                                volume_db = -144;
                            }
                            if (volume_db > 0) {
                                volume_db = 0;
                            }
                            if (volume_db == -144) {
                                volume_p = 0;
                            } else {
                                volume_p = 1.0 + volume_db / 30.0;
                            }
                            
                            log_message(LOG_INFO, "Client set volume: %f (%f)", volume_p, volume_db);
                            
                            audio_output_set_volume(audio_queue_get_output(rtp_session->queue), volume_p);
                            if (rs->callbacks.updated_volume != NULL) {
                                rs->callbacks.updated_volume(rs, volume_p, rs->callbacks.ctx.updated_volume);
                            }
                            
                        } else {
                            
                            log_message(LOG_INFO, "Ignored set volume.");
                            
                        }
                        
                    } else if (rs->callbacks.updated_track_position != NULL && (progress = parameters_value_for_key(parameters, "progress"))) {
                        
                        unsigned int start, curr, end;
                        sscanf(progress, "%u/%u/%u", &start, &curr, &end);
                        
                        log_message(LOG_INFO, "Client set progress (%s): %u/%u/%u", progress, start, curr, end);
                        
                        struct decoder_output_format_t output_format = decoder_get_output_format(rs->decoder);
                        
                        double srate = (double)output_format.sample_rate;
                        if (srate == 0.0) {
                            srate = 44100;
                        }
                        double position = (double)(curr - start) / srate;
                        double total = (double)(end - start) / srate;
                        rs->total_length = total;
                        rs->start_rtp_timestamp = start;
                        if (rs->callbacks.updated_track_position != NULL) {
                            rs->callbacks.updated_track_position(rs, position, total, rs->callbacks.ctx.updated_track_position);
                        }
                        
                    }
                    
                } else {
                    
                    const char* mime_type = web_headers_value(request_headers, "Content-Type");
                    
                    size_t data_size = web_request_get_content(request, NULL, 0);
                    char* data = (char*)malloc(data_size);
                    web_request_get_content(request, data, data_size);
                    
                    if (rs->callbacks.updated_track_info != NULL && 0 == strcmp(mime_type, "application/x-dmap-tagged")) {
                        
                        dmap_p tags = dmap_create();
                        dmap_parse(tags, data, data_size);
                        
                        uint32_t container_tag;
                        if (dmap_get_count(tags) > 0 && dmap_type_for_tag((container_tag = dmap_get_tag_at_index(tags, 0))) == dmap_type_container) {
                            
                            dmap_p track_tags = dmap_container_for_atom_tag(tags, container_tag);
                            
                            const char* title = dmap_string_for_atom_identifer(track_tags, "dmap.itemname");
                            const char* artist = dmap_string_for_atom_identifer(track_tags, "daap.songartist");
                            const char* album = dmap_string_for_atom_identifer(track_tags, "daap.songalbum");
                            
                            rs->callbacks.updated_track_info(rs, title, artist, album, rs->callbacks.ctx.updated_track_info);
                            
                        }
                        
                        object_release(tags);
                        
                    } else if (rs->callbacks.updated_artwork != NULL)
                        rs->callbacks.updated_artwork(rs, data, data_size, mime_type, rs->callbacks.ctx.updated_artwork);
                    
                    free(data);
                    
                }
                
            } else if (0 == strcmp(cmd, "FLUSH") && rtp_session != NULL) {
                
                uint16_t last_seq = 0;
                const char* rtp_info;
                if ((rtp_info = web_headers_value(request_headers, "RTP-Info")) != NULL) {
                    
                    parameters_p rtp_params = parameters_create(rtp_info, strlen(rtp_info), parameters_type_http_header);
                    
                    const char* seq;
                    if ((seq = parameters_value_for_key(rtp_params, "seq")) != NULL)
                        last_seq = atoi(seq);
                    
                    object_release(rtp_params);
                    
                }
                
                if (rs->dacp_client != NULL)
                    dacp_client_update_playback_state(rs->dacp_client);
                
                audio_queue_flush(rtp_session->queue, last_seq);
                
            } else if (0 == strcmp(cmd, "TEARDOWN") && rtp_session != NULL) {
                
                if (rs->callbacks.ended_recording != NULL)
                    rs->callbacks.ended_recording(rs, rs->callbacks.ctx.ended_recording);
                
                keep_alive = false;
                
            } else
                web_response_set_status(response, 400, "Bad Request");
            
        } else {
            
            mutex_lock(rs->mutex);
            
            char nonce[16];
            
            for (uint32_t i = 0 ; i < 16 ; i++)
                nonce[i] = (char) rand() % 256;
            
            hex_encode(nonce, 16, rs->authentication_digest_nonce, 32);
            rs->authentication_digest_nonce[32] = '\0';
            
            web_headers_set_value(response_headers, "WWW-Authenticate", "Digest realm=\"raop\", nonce=\"%s\"", rs->authentication_digest_nonce);
            
            mutex_unlock(rs->mutex);
            
            web_response_set_status(response, 401, "Unauthorized");
            
        }
        
        const char* challenge;
        if ((challenge = web_headers_value(request_headers, "Apple-Challenge"))) {
            
            size_t a_res_size = 1000;
            char a_res[a_res_size];
            
            size_t challenge_length = strlen(challenge);
            char r_challange[challenge_length + 5];
            base64_pad(challenge, challenge_length, r_challange, challenge_length + 5);
            _raop_session_get_apple_response(rs, r_challange, strlen(r_challange), a_res, &a_res_size);
            
            if (a_res_size > 0) {
                a_res[a_res_size] = '\0';
                web_headers_set_value(response_headers, "Apple-Response", "%s", a_res);
            }
            
        }
        
        web_headers_set_value(response_headers, "Audio-Jack-Status", "connected; type=digital");
        
        if (parameters != NULL) {
            object_release(parameters);
            parameters = NULL;
        }
        
    } else
        web_response_set_status(response, 400, "Bad Request");
    
    web_server_connection_send_response(rs->raop_connection, response, "RTSP/1.0", !keep_alive);
    
    object_release(response);
    
}

void _raop_session_raop_closed_callback(web_server_connection_p connection, void* ctx) {
    
    struct raop_session_t* rs = (struct raop_session_t*)ctx;
    
    raop_session_stop(rs);
    
}

void _raop_session_destroy(void* object) {
    
    struct raop_session_t* rs = (struct raop_session_t*)object;
    
    mutex_lock(rs->mutex);
    
    if (rs->is_running) {
        mutex_unlock(rs->mutex);
        raop_session_stop(rs);
        mutex_lock(rs->mutex);
    }
    
    if (rs->password != NULL) {
        free(rs->password);
        rs->password = NULL;
    }
    
    object_release(rs->server);
    object_release(rs->raop_connection);
    
    mutex_unlock(rs->mutex);
    
    mutex_destroy(rs->mutex);
    rs->mutex = NULL;
    
}

struct raop_session_t* raop_session_create(raop_server_p server, web_server_connection_p connection, settings_p settings) {
    
    struct raop_session_t* rs = (struct raop_session_t*)object_create(sizeof(struct raop_session_t), _raop_session_destroy);
    
    rs->server = (raop_server_p)object_retain(server);
    rs->raop_connection = (web_server_connection_p)object_retain(connection);
    rs->total_length = 0;
    rs->start_rtp_timestamp = 0;
    
    const char* password = settings_get_password(settings);
    if (password != NULL && strlen(password) > 0) {
        rs->password = (char*)malloc(strlen(password) + 1);
        strcpy(rs->password, password);
    }
    
    rs->ignore_source_volume = settings_get_ignore_source_volume(settings);
    
    web_server_connection_set_request_callback(rs->raop_connection, _raop_session_raop_connection_request_callback, rs);
    web_server_connection_set_closed_callback(rs->raop_connection, _raop_session_raop_closed_callback, rs);
    
    rs->mutex = mutex_create();
    
    return rs;
    
}

void raop_session_start(struct raop_session_t* rs) {
    
    mutex_lock(rs->mutex);
    
    if (!rs->is_running) {
        rs->is_running = true;
    }
    
    mutex_unlock(rs->mutex);
    
}

void raop_session_stop(struct raop_session_t* rs) {
    
    bool stopped = false;
    
    mutex_lock(rs->mutex);
    
    if (rs->is_running) {
        
        web_server_connection_close(rs->raop_connection);
        rs->is_running = false;
        stopped = true;
        
        if (rs->callbacks.ended != NULL) {
            mutex_unlock(rs->mutex);
            rs->callbacks.ended(rs, rs->callbacks.ctx.ended);
            mutex_lock(rs->mutex);
        }
        
    }
    
    if (rs->rtp_session != NULL){
        object_release(rs->rtp_session->recorder);
        object_release(rs->rtp_session->queue);
        free(rs->rtp_session);
        rs->rtp_session = NULL;
    }
    
    if (rs->decoder != NULL) {
        object_release(rs->decoder);
        rs->decoder = NULL;
    }
    
    if (rs->crypt_aes != NULL) {
        object_release(rs->crypt_aes);
        rs->crypt_aes = NULL;
    }
    
    if (rs->dacp_client != NULL) {
        object_release(rs->dacp_client);
        rs->dacp_client = NULL;
    }
    
    if (rs->user_agent != NULL) {
        free(rs->user_agent);
        rs->user_agent = NULL;
    }
    
    mutex_unlock(rs->mutex);
    
    if (stopped)
        raop_server_session_ended(rs->server, rs);
        
}

void raop_session_set_client_initiated_callback(struct raop_session_t* rs, raop_session_client_initiated_callback callback, void* ctx) {
    
    rs->callbacks.initiated = callback;
    rs->callbacks.ctx.initiated = ctx;
    
}

void raop_session_set_client_started_recording_callback(struct raop_session_t* rs, raop_session_client_started_recording_callback callback, void* ctx) {
    
    rs->callbacks.started_recording = callback;
    rs->callbacks.ctx.started_recording = ctx;
    
}

void raop_session_set_client_updated_track_info_callback(struct raop_session_t* rs, raop_session_client_updated_track_info_callback callback, void* ctx) {
    
    rs->callbacks.updated_track_info = callback;
    rs->callbacks.ctx.updated_track_info = ctx;
    
}

void raop_session_set_client_updated_track_position_callback(struct raop_session_t* rs, raop_session_client_updated_track_position_callback callback, void* ctx) {
    
    rs->callbacks.updated_track_position = callback;
    rs->callbacks.ctx.updated_track_position = ctx;
    
}

void raop_session_set_client_updated_artwork_callback(struct raop_session_t* rs, raop_session_client_updated_artwork_callback callback, void* ctx) {
    
    rs->callbacks.updated_artwork = callback;
    rs->callbacks.ctx.updated_artwork = ctx;
    
}

void raop_session_set_client_updated_volume_callback(raop_session_p rs, raop_session_client_updated_volume_callback callback, void* ctx) {
    
    rs->callbacks.updated_volume = callback;
    rs->callbacks.ctx.updated_volume = ctx;
    
}

void raop_session_set_client_ended_recording_callback(struct raop_session_t* rs, raop_session_client_ended_recording_callback callback, void* ctx) {
    
    rs->callbacks.ended_recording = callback;
    rs->callbacks.ctx.ended_recording = ctx;
    
}

void raop_session_set_ended_callback(struct raop_session_t* rs, raop_session_ended_callback callback, void* ctx) {
    
    rs->callbacks.ended = callback;
    rs->callbacks.ctx.ended = ctx;
    
}

bool raop_session_is_recording(struct raop_session_t* rs) {
    
    mutex_lock(rs->mutex);
    bool ret = (rs->rtp_session != NULL);
    mutex_unlock(rs->mutex);
    
    return ret;
    
}

dacp_client_p raop_session_get_dacp_client(struct raop_session_t* rs) {
    
    return rs->dacp_client;
    
}

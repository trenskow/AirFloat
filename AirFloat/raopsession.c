//
//  raopsession.c
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
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
#include "settings.h"
#include "hardware.h"
#include "notificationcenter.h"

#include "parameters.h"
#include "dmap.h"

#include "webtools.h"
#include "webconnection.h"

#include "dacpclient.h"

#include "crypt.h"
#include "decoder.h"

#include "audioqueue.h"
#include "audiooutput.h"
#include "raopserver.h"
#include "rtprecorder.h"

#include "raopsession.h"

#define MAX(x,y) (x > y ? x : y)

const char* raop_session_client_setup_recorder_notification = "raop_session_client_setup_recorder_notification";
const char* raop_session_client_recording_started_notification = "raop_session_client_recording_started_notification";
const char* raop_session_client_updated_progress_notification = "raop_session_client_updated_progress_notification";
const char* raop_session_client_updated_track_notification = "raop_session_client_updated_track_notification";
const char* raop_session_client_updated_artwork_notification = "raop_session_client_updated_artwork_notification";
const char* raop_session_client_torned_down_recorder_notification = "raop_session_client_torned_down_recorder_notification";

struct raop_rtp_session_t {
    audio_queue_p queue;
    rtp_recorder_p recorder;
    uint32_t session_id;
};

struct raop_session_t {
    mutex_p mutex;
    bool started;
    bool stopped;
    raop_server_p server;
    web_connection_p raop_connection;
    char authentication_digest_nonce[33];
    dacp_client_p dacp_client;
    char* user_agent;
    crypt_aes_p crypt_aes;
    decoder_p decoder;
    struct raop_rtp_session_t* rtp_sessions;
    uint32_t rtp_sessions_count;
    uint32_t rtp_last_session_id;
    raop_session_ended_callback ended_callback;
    void* ended_callback_ctx;
};

bool _raop_session_check_authentication(struct raop_session_t* rs, const char* method, const char* uri, const char* authentication_parameter) {
    
    assert(method != NULL && uri != NULL);
    
    bool ret = true;
    
    const char* password = settings_get_password();
    
    if (password != NULL) {
        
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
                    size_t pw_len = strlen(password);
                    
                    char a1pre[strlen(username) + strlen(realm) + pw_len + 3];
                    sprintf(a1pre, "%s:%s:%s", username, realm, password);
                    
                    char a2pre[strlen(method) + strlen(uri) + 2];
                    sprintf(a2pre, "%s:%s", method, uri);
                    
                    uint16_t a1[16], a2[16];
                    crypt_md5_hash(a1pre, strlen(a1pre), a1, 16);
                    crypt_md5_hash(a2pre, strlen(a2pre), a2, 16);
                    
                    char ha1[33], ha2[33];
                    ha1[32] = ha2[32] = '\0';
                    crypt_hex_encode(a1, 16, ha1, 32);
                    crypt_hex_encode(a2, 16, ha2, 32);
                    
                    char finalpre[67 + strlen(rs->authentication_digest_nonce)];
                    sprintf(finalpre, "%s:%s:%s", ha1, rs->authentication_digest_nonce, ha2);
                    
                    uint16_t final[16];
                    crypt_md5_hash(finalpre, strlen(finalpre), final, 16);
                    
                    char hfinal[33];
                    hfinal[32] = '\0';
                    crypt_hex_encode(final, 32, hfinal, 32);
                    
                    for (int i = 0 ; i < 32 ; i++) {
                        hfinal[i] = tolower(hfinal[i]);
                        w_response[i] = tolower(w_response[i]);
                    }
                    
                    if (strcmp(hfinal, w_response) != 0) {
                        log_message(LOG_INFO, "Authentication failure");
                        ret = false;
                    }
                    
                }
                
                parameters_destroy(parameters);
                
            }
            
        } else
            log_message(LOG_INFO, "Authentication header missing");
        
    }
    
    return ret;
    
}

void _raop_session_get_apple_response(struct raop_session_t* rs, const char* challenge, size_t challenge_length, char* response, size_t* response_length) {
    
    char decoded_challenge[1000];
    size_t actual_length = base64_decode(challenge, decoded_challenge);
    
    if (actual_length != 16)
        log_message(LOG_ERROR, "Apple-Challenge: Expected 16 bytes - got %d", actual_length);
    
    struct sockaddr* local_end_point = web_connection_get_local_end_point(rs->raop_connection);
    uint64_t hw_identifier = hardware_identifier();
    
    size_t response_size = 32;
    char a_response[48]; // IPv6 responds with 48 bytes
    
    memset(a_response, 0, sizeof(a_response));
    
    if (local_end_point->sa_family == AF_INET6) {
        
        response_size = 48;
        
        memcpy(a_response, decoded_challenge, actual_length);
        memcpy(&a_response[actual_length], &((struct sockaddr_in6*)local_end_point)->sin6_addr, 16);
        memcpy(&a_response[actual_length + 16], &((char*)&hw_identifier)[2], 6);
        
    } else {
        
        memcpy(a_response, decoded_challenge, actual_length);
        memcpy(&a_response[actual_length], &((struct sockaddr_in*)local_end_point)->sin_addr.s_addr, 4);
        memcpy(&a_response[actual_length + 4], &((char*)&hw_identifier)[2], 6);
        
    }
    
    unsigned char clearResponse[256];
    memset(clearResponse, 0xFF, 256);
    clearResponse[0] = 0;
    clearResponse[1] = 1;
    clearResponse[256 - (response_size + 1)] = 0;
    memcpy(&clearResponse[256 - response_size], a_response, response_size);
    
    unsigned char encryptedResponse[256];
    size_t size = crypt_apple_private_encrypt(clearResponse, 256, encryptedResponse, 256);
    
    if (size > 0) {
        
        char* a_encrypted_response;
        size_t a_len = base64_encode(encryptedResponse, size, &a_encrypted_response);
        
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

void _raop_session_raop_connection_request_callback(web_connection_p connection, web_request_p request, void* ctx) {
    
    struct raop_session_t* rs = (struct raop_session_t*)ctx;
    
    mutex_lock(rs->mutex);
    
    int c_seq = 0;
    
    const char* cmd = web_request_get_command(request);
    const char* path = web_request_get_path(request);
    web_response_p response = web_request_get_response(request);
    web_headers_p request_headers = web_request_get_headers(request);
    web_headers_p response_headers = web_response_get_headers(response);
    
    web_response_set_status(response, 200, "OK");
    web_response_set_keep_alive(response, true);
    
    struct raop_rtp_session_t* rtp_session = NULL;
    
    if (cmd != NULL && path != NULL) {
        
        parameters_p parameters = NULL;
        
        size_t content_length;
        if ((content_length = web_request_get_content(request, NULL, 0)) > 0) {
            const char* contentType = web_headers_value(request_headers, "Content-Type");
            if (strcmp(contentType, "application/sdp") == 0 || strcmp(contentType, "text/parameters") == 0) {
                
                char* content[content_length];
                web_request_get_content(request, content, content_length);
                
                size_t cl = web_tools_convert_new_lines(content, content_length);
                if (strcmp(contentType, "application/sdp") == 0)
                    parameters = parameters_create(content, cl, parameters_type_sdp);
                else if (strcmp(contentType, "text/parameters") == 0)
                    parameters = parameters_create(content, cl, parameters_type_text);
                
                log_data(LOG_INFO, (char*)content, content_length);
                
            }
        }
        
        const char *user_agent;
        if (rs->user_agent == NULL && (user_agent = web_headers_value(request_headers, "User-Agent")) != NULL) {
            rs->user_agent = (char*)malloc(strlen(user_agent) + 1);
            strcpy(rs->user_agent, user_agent);
        }
        
        web_headers_set_value(response_headers, "Server", "AirTunes/105.1");
        web_headers_set_value(response_headers, "CSeq", "%d", c_seq);
        
        const char *session_id;
        if ((session_id = web_headers_value(request_headers, "Session")) != NULL) {
            uint32_t session = atoi(session_id);
            for (uint32_t i = 0 ; i < rs->rtp_sessions_count ; i++)
                if (rs->rtp_sessions[i].session_id == session) {
                    rtp_session = &rs->rtp_sessions[i];
                    web_headers_set_value(response_headers, "Session", "%d", rtp_session->session_id);
                    break;
                }
            
        }
        
        if (_raop_session_check_authentication(rs, cmd, path, web_headers_value(request_headers, "Authorization"))) {
            
            if (0 == strcmp(cmd, "OPTIONS"))
                web_headers_set_value(response_headers, "Public", "ANNOUNCE, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, GET_PARAMETER, SET_PARAMETER, POST, GET");
            else if (0 == strcmp(cmd, "ANNOUNCE")) {
                
                if (rs->dacp_client == NULL) {
                    
                    const char* dacp_id;
                    const char* active_remote;
                    if ((dacp_id = web_headers_value(request_headers, "DACP-ID")) != NULL && (active_remote = web_headers_value(request_headers, "Active-Remote")) != NULL)
                        rs->dacp_client = dacp_client_create(dacp_id, active_remote);
                    
                }
                
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
                        
                        rs->decoder = decoder_create(codec, rtp_fmtp);
                        
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
                            
                            rs->crypt_aes = crypt_aes_create(aes_key, aes_initializer, size);
                            
                        }
                        
                    } else
                        web_response_set_status(response, 400, "Bad Request");
                    
                } else
                    web_response_set_status(response, 400, "Bad Request");
                
            } else if (0 == strcmp(cmd, "SETUP")) {
                
                mutex_unlock(rs->mutex);
                bool is_recorder = raop_server_is_recording(rs->server);
                mutex_lock(rs->mutex);
                
                if (!is_recorder) {
                    
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
                        
                        audio_queue_p audio_queue = audio_queue_create(rs->decoder);
                        
                        struct sockaddr* local_end_point = web_connection_get_local_end_point(rs->raop_connection);
                        struct sockaddr* remote_end_point = web_connection_get_remote_end_point(rs->raop_connection);
                        rtp_recorder_p new_recorder = rtp_recorder_create(rs->crypt_aes, audio_queue, local_end_point, remote_end_point, control_port, timing_port);
                        
                        uint32_t session_id = ++rs->rtp_last_session_id;
                        
                        rs->rtp_sessions = (struct raop_rtp_session_t*)realloc(rs->rtp_sessions, sizeof(struct raop_rtp_session_t) + (rs->rtp_sessions_count + 1));
                        rs->rtp_sessions[rs->rtp_sessions_count] = (struct raop_rtp_session_t){ audio_queue, new_recorder, session_id };
                        rtp_session = &rs->rtp_sessions[rs->rtp_sessions_count];
                        rs->rtp_sessions_count++;
                        
                        web_headers_set_value(response_headers, "Session", "%d", session_id);
                        
                        parameters_set_value(transport_params, "timing_port", "%d", rtp_recorder_get_timing_port(rtp_session->recorder));
                        parameters_set_value(transport_params, "control_port", "%d", rtp_recorder_get_control_port(rtp_session->recorder));
                        parameters_set_value(transport_params, "server_port", "%d", rtp_recorder_get_streaming_port(rtp_session->recorder));
                        
                        size_t transport_reply_len = parameters_write(transport_params, NULL, 0, parameters_type_http_header);
                        char transport_reply[transport_reply_len];
                        parameters_write(transport_params, transport_reply, transport_reply_len, parameters_type_http_header);
                        
                        web_headers_set_value(response_headers, "Transport", transport_reply);
                        
                        mutex_unlock(rs->mutex);
                        notification_center_post_notification(raop_session_client_setup_recorder_notification, rs, NULL);
                        mutex_lock(rs->mutex);
                        
                        if (parameters != NULL)
                            parameters_destroy(parameters);
                        
                    } else
                        web_response_set_status(response, 400, "Bad Request");
                    
                } else
                    web_response_set_status(response, 453, "Not Enough Bandwidth");
                
            } else if (0 == strcmp(cmd, "RECORD") && rtp_session != NULL) {
                
                if (rtp_recorder_start(rtp_session->recorder)) {
                    
                    audio_queue_start(rtp_session->queue);
                    
                    mutex_unlock(rs->mutex);
                    notification_center_post_notification(raop_session_client_recording_started_notification, rs, NULL);
                    mutex_lock(rs->mutex);
                    
                    web_headers_set_value(response_headers, "Audio-Latency", "11025");
                    
                } else
                    web_response_set_status(response, 400, "Bad Request");
                
            } else if (0 == strcmp(cmd, "SET_PARAMETER") && rtp_session != NULL) {
                
                if (parameters != NULL) {
                    
                    const char* volume;
                    const char* progress;
                    if ((volume = parameters_value_for_key(parameters, "volume"))) {
                        float f_volume;
                        sscanf(volume, "%f", &f_volume);
                        log_message(LOG_INFO, "Client set volume: %f", (30.0 + f_volume) / 30.0);
                        
                        audio_output_set_volume(audio_queue_get_output(rtp_session->queue), MAX(pow(10.0, 0.05 * f_volume), 0.0));
                        
                    } else if ((progress = parameters_value_for_key(parameters, "progress"))) {
                        
                        const char* parts[3] = { NULL, NULL, NULL };
                        uint32_t c_art = 1;
                        parts[0] = progress;
                        
                        for (size_t i = 0 ; i < strlen(progress) ; i++)
                            if (progress[i] == '/')
                                parts[c_art++] = &progress[i+1];
                        
                        double start = atoi(parts[0]);
                        
                        struct decoder_output_format_t output_format = decoder_get_output_format(rs->decoder);
                        
                        struct raop_session_client_updated_progress_info info;
                        
                        info.position = (atoi(parts[1]) - start) / (double)output_format.sample_rate;
                        info.total = (atoi(parts[2]) - start) / (double)output_format.sample_rate;
                        
                        mutex_unlock(rs->mutex);
                        notification_center_post_notification(raop_session_client_updated_progress_notification, rs, &info);
                        mutex_lock(rs->mutex);
                        
                    }
                    
                } else {
                    
                    const char* mime_type = web_headers_value(request_headers, "Content-Type");
                    
                    size_t data_size = web_request_get_content(request, NULL, 0);
                    char* data = (char*)malloc(data_size);
                    web_request_get_content(request, data, data_size);
                    
                    if (0 == strcmp(mime_type, "application/x-dmap-tagged")) {
                        
                        dmap_p tags = dmap_create();
                        dmap_parse(tags, data, data_size);
                        
                        uint32_t container_tag;
                        if (dmap_get_count(tags) > 0 && dmap_type_for_tag((container_tag = dmap_get_tag_at_index(tags, 0))) == dmap_type_container) {
                            
                            dmap_p track_tags = dmap_container_for_atom_tag(tags, container_tag);
                            mutex_unlock(rs->mutex);
                            notification_center_post_notification(raop_session_client_updated_track_notification, rs, track_tags);
                            mutex_lock(rs->mutex);
                            
                            const char* title = dmap_string_for_atom_identifer(track_tags, "dmap.itemname");
                            const char* artist = dmap_string_for_atom_identifer(track_tags, "dmap.songartist");
                            if (title != NULL && artist != NULL)
                                log_message(LOG_INFO, "Now playing: \"%s\" by %s", title, artist);
                            
                        }
                        
                        dmap_destroy(tags);
                        
                    } else {
                        
                        log_message(LOG_INFO, "Track info received (Content-Type: %s)", mime_type);
                        
                        struct raop_session_client_updated_artwork_info info;
                        
                        info.data = data;
                        info.data_size = data_size;
                        info.mime_type = mime_type;
                        
                        mutex_unlock(rs->mutex);
                        notification_center_post_notification(raop_session_client_updated_artwork_notification, rs, &info);
                        mutex_lock(rs->mutex);
                        
                    }
                    
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
                    
                    parameters_destroy(rtp_params);
                    
                }
                
                mutex_unlock(rs->mutex);
                audio_queue_flush(rtp_session->queue, last_seq);
                mutex_lock(rs->mutex);
                
            } else if (0 == strcmp(cmd, "TEARDOWN") && rtp_session != NULL) {
                
                audio_queue_destroy(rtp_session->queue);
                rtp_recorder_destroy(rtp_session->recorder);
                
                for (uint32_t i = 0 ; i < rs->rtp_sessions_count ; i++)
                    if (rs->rtp_sessions[i].session_id == rtp_session->session_id) {
                        for (uint32_t a = i + 1 ; a < rs->rtp_sessions_count ; a++)
                            rs->rtp_sessions[i - 1] = rs->rtp_sessions[i];
                        break;
                    }
                
                mutex_unlock(rs->mutex);
                notification_center_post_notification(raop_session_client_torned_down_recorder_notification, rs, NULL);
                mutex_lock(rs->mutex);
                
            } else
                web_response_set_status(response, 400, "Bad Request");
            
        } else {
            
            char nonce[16];
            
            for (uint32_t i = 0 ; i < 16 ; i++)
                nonce[i] = (char) rand() % 256;
            
            crypt_hex_encode(nonce, 16, rs->authentication_digest_nonce, 32);
            rs->authentication_digest_nonce[32] = '\0';
            
            web_headers_set_value(response_headers, "WWW-Authenticate", "Digest realm=\"raop\", nonce=\"%s\"", rs->authentication_digest_nonce);
            
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
        
        if (parameters != NULL)
            parameters_destroy(parameters);
        
    } else
        web_response_set_status(response, 400, "Bad Request");
    
    mutex_unlock(rs->mutex);
    
}

void _raop_session_raop_closed_callback(web_connection_p connection, void* ctx) {
    
    struct raop_session_t* rs = (struct raop_session_t*)ctx;
    
    raop_session_stop(rs);
    
}

struct raop_session_t* raop_session_create(raop_server_p server, web_connection_p connection) {
    
    struct raop_session_t* rs = (struct raop_session_t*)malloc(sizeof(struct raop_session_t));
    bzero(rs, sizeof(struct raop_session_t));
    
    rs->server = server;
    rs->raop_connection = connection;
    
    web_connection_set_request_callback(rs->raop_connection, _raop_session_raop_connection_request_callback, rs);
    web_connection_set_closed_callback(rs->raop_connection, _raop_session_raop_closed_callback, rs);
    
    rs->mutex = mutex_create();
    
    return rs;
    
}

void _raop_session_reset(struct raop_session_t* rs) {
    
    if (rs->started && !rs->stopped)
        web_connection_close(rs->raop_connection);
    
    if (rs->dacp_client != NULL) {
        dacp_client_destroy(rs->dacp_client);
        rs->dacp_client = NULL;
    }
    
    if (rs->user_agent != NULL) {
        free(rs->user_agent);
        rs->user_agent = NULL;
    }

}

void raop_session_destroy(struct raop_session_t* rs) {
    
    mutex_lock(rs->mutex);
    
    _raop_session_reset(rs);
    
    mutex_unlock(rs->mutex);
    
    mutex_destroy(rs->mutex);
    
    free(rs);
    
}

void raop_session_start(struct raop_session_t* rs) {
    
    mutex_lock(rs->mutex);
    
    if (!rs->started)
        rs->started = true;
    
    mutex_unlock(rs->mutex);
    
}

void raop_session_stop(struct raop_session_t* rs) {
    
    mutex_lock(rs->mutex);
    
    bool closed = false;
    
    if (rs->started && !rs->stopped) {
        
        _raop_session_reset(rs);
        
        closed = rs->stopped = true;
        closed = true;
        
    }
    
    mutex_unlock(rs->mutex);
    
    if (closed && rs->ended_callback != NULL)
        rs->ended_callback(rs, rs->ended_callback_ctx);
    
}

void raop_session_set_ended_callback(struct raop_session_t* rs, raop_session_ended_callback callback, void* ctx) {
    
    rs->ended_callback = callback;
    rs->ended_callback_ctx = ctx;
    
}

bool raop_session_is_recording(struct raop_session_t* rs) {
    
    mutex_lock(rs->mutex);
    bool ret = (rs->rtp_sessions_count > 0);
    mutex_unlock(rs->mutex);
    
    return ret;
    
}

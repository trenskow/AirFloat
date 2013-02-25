
//
//  RAOPConnection.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <stdio.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if_dl.h>

#include <AudioToolbox/AudioToolbox.h>

#include "RAOPParameters.h"
#include "RAOPConnection.h"

extern "C" {
#include "log.h"
#include "settings.h"
#include "notificationcenter.h"
#include "dmap.h"
#include "base64.h"
#include "webconnection.h"
#include "webrequest.h"
}

#define AIRPORT_PRIVATE_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\n" \
"wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\n" \
"wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\n" \
"/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\n" \
"UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\n" \
"BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\n" \
"LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\n" \
"NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\n" \
"lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\n" \
"aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\n" \
"a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\n" \
"oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\n" \
"oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\n" \
"k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\n" \
"AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\n" \
"cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\n" \
"54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\n" \
"17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\n" \
"1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\n" \
"LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\n" \
"2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKaXTyY=\n" \
"-----END RSA PRIVATE KEY-----"

#define MIN(x, y) (x < y ? x : y)

static char* addToBuffer(char* buffer, const char* format, ...) {
    
    char buf[1000];
    
    va_list args;
    va_start(args, format);
    vsnprintf(buf, 1000, format, args);
    va_end(args);
    
    long bufLength = strlen(buf);
    
    buffer = (char*)realloc(buffer, strlen(buffer) + bufLength + 2);
    memcpy(&buffer[strlen(buffer)], buf, bufLength + 1);
    
    return buffer;
    
}

static char* interfaceForIp(struct sockaddr* addr) {
    
    static char ret[100];
    
    struct ifaddrs* if_addrs = NULL;
    struct ifaddrs* if_addr = NULL;
    
    if (0 == getifaddrs(&if_addrs)) {
        
        for (if_addr = if_addrs ; if_addr != NULL ; if_addr = if_addr->ifa_next) {
            
            unsigned char ip[16];
            memset(ip, 0, 16);
            if (if_addr->ifa_addr->sa_family == addr->sa_family) {
                
                if ((addr->sa_family == AF_INET && ((struct sockaddr_in*)addr)->sin_addr.s_addr == ((struct sockaddr_in*)if_addr->ifa_addr)->sin_addr.s_addr) 
                    ||
                    (addr->sa_family == AF_INET6 && memcmp(&((struct sockaddr_in6*)addr)->sin6_addr, &((struct sockaddr_in6*)if_addr->ifa_addr)->sin6_addr, 16) == 0))  {
                    
                    strcpy(ret, if_addr->ifa_name);
                    freeifaddrs(if_addrs);
                    return ret;
                    
                }
                
            }
                        
        }
        
    }
    
    freeifaddrs(if_addrs);
    
    return NULL;
    
}

static char* hwAddressForInterface(const char* interface) {
    
    static char ret[6];
    
    struct ifaddrs* if_addrs = NULL;
    struct ifaddrs* if_addr = NULL;
    
    if (0 == getifaddrs(&if_addrs))        
        for (if_addr = if_addrs ; if_addr != NULL ; if_addr = if_addr->ifa_next) {
            
            if (if_addr->ifa_name != NULL && if_addr->ifa_addr->sa_family == AF_LINK && strcmp(interface, if_addr->ifa_name) == 0) {
                
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)if_addr->ifa_addr;
                if (sdl->sdl_alen == 6) {
                    memcpy(ret, LLADDR(sdl), sdl->sdl_alen);
                    freeifaddrs(if_addrs);
                    return ret;
                }
                
            }
            
        }
    
    freeifaddrs(if_addrs);
    
    return NULL;
    
}

static long padBase64(const char* base64, long length, char* out) {
    
    memcpy(out, base64, length);
    for (long i = 0 ; i < (length % 4) ; i++)
        out[length + i] = '=';
    out[length + (length % 4)] = '\0';
    
    return length + (length % 4);
    
}

static void md5ToString(unsigned char* md5, char* out) {
    
    char hexValues[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    
    out[32] = '\0';
    for (int i = 0 ; i < 16 ; i++) {
        out[i*2] = hexValues[(md5[i] >> 4) & 0xF];
        out[(i*2)+1] = hexValues[md5[i] & 0xF];
    }
    
}

static void md5(const char* content, size_t length, unsigned char* md5) {
    
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, content, length);
    MD5_Final(md5, &context);
    
}

const char* RAOPConnection::recordingStartedNotificationName = "connectionRecordingStarted";
const char* RAOPConnection::clientConnectedNotificationName = "connectionConnectionCreated";
const char* RAOPConnection::clientDisconnectedNotificationName = "connectionClientDisconnected";
const char* RAOPConnection::clientUpdatedMetadataNotificationName = "connectionClientUpdatedMetadata";
const char* RAOPConnection::clientUpdatedProgressNotificationName = "connectionClientUpdatedProgress";
const char* RAOPConnection::clientUpdatedTrackInfoNofificationName = "connectionClientUpdatedTrackInfo";

RAOPConnection::RAOPConnection(web_connection_p connection) {
        
    _connection = connection;
    
    web_connection_set_request_callback(_connection, _processRequestCallbackHelper, this);
    web_connection_set_close_callback(_connection, _connectionClosedCallbackHelper, this);
    
    _rtp = NULL;
    
    BIO* bio = BIO_new(BIO_s_mem());
    BIO_write(bio, AIRPORT_PRIVATE_KEY, strlen(AIRPORT_PRIVATE_KEY));
    _applePrivateKey = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    
    const char *password;
    
    if ((password = settings_get_password())) {
        _password = (char*)malloc(strlen(password) + 1);
        strcpy(_password, password);
    } else
        _password = NULL;

    memset(_digestNonce, 0, 33);
    
    _dacpId[0] = _activeRemote[0] = _sessionId[0] = _userAgent[0] = '\0';
    
}

RAOPConnection::~RAOPConnection() {
    
    if (_rtp != NULL) {
        _rtp->stop();
        delete _rtp;
        _rtp = NULL;
        notification_center_post_notification(RAOPConnection::clientDisconnectedNotificationName, this, NULL);
    }
    
    if (_password != NULL)
        free(_password);
    
    RSA_free(_applePrivateKey);
    
    log_message(LOG_INFO, "Connection cleanup complete");
    
}

RTPReceiver* RAOPConnection::getRTPReceiver() {
    
    return _rtp;
    
}

struct sockaddr* RAOPConnection::getLocalEndPoint() {
    
    return web_connection_get_local_end_point(_connection);
    
}

struct sockaddr* RAOPConnection::getRemoteEndPoint() {
    
    return web_connection_get_remote_end_point(_connection);
    
}

void RAOPConnection::_appleResponse(const char* challenge, long length, char* response, long* resLength) {
    
    unsigned char decodedChallenge[1000];
    int actualLength = base64_decode(challenge, decodedChallenge);
    
    if (actualLength != 16)
        log_message(LOG_ERROR, "Apple-Challenge: Expected 16 bytes - got %d", actualLength);
    
    struct sockaddr* localEndPoint = web_connection_get_local_end_point(_connection);
    char* interface = interfaceForIp(localEndPoint);
    
    if (interface != NULL) {
        
        int responseSize = 32;
        char aResponse[48]; // IPv6 responds with 48 bytes
        
        char* hwid = hwAddressForInterface(interface);
        memset(aResponse, 0, sizeof(aResponse));
        
        if (hwid != NULL) {
            
            if (localEndPoint->sa_family == AF_INET6) {
                
                responseSize = 48;
                
                memcpy(aResponse, decodedChallenge, actualLength);
                memcpy(&aResponse[actualLength], &((struct sockaddr_in6*)localEndPoint)->sin6_addr, 16);
                memcpy(&aResponse[actualLength + 16], hwid, 6);
                
            } else {
                
                memcpy(aResponse, decodedChallenge, actualLength);
                memcpy(&aResponse[actualLength], &((struct sockaddr_in*)localEndPoint)->sin_addr.s_addr, 4);
                memcpy(&aResponse[actualLength + 4], hwid, 6);
                
            }
            
        } else
            log_message(LOG_ERROR, "Error getting hardware ID");
        
        int size = RSA_size(_applePrivateKey);
        unsigned char clearResponse[size];
        memset(clearResponse, 0xFF, size);
        clearResponse[0] = 0;
        clearResponse[1] = 1;
        clearResponse[size - (responseSize + 1)] = 0;
        memcpy(&clearResponse[size - responseSize], aResponse, responseSize);
        
        unsigned char encryptedResponse[size];
        size = RSA_private_encrypt(size, clearResponse, encryptedResponse, _applePrivateKey, RSA_NO_PADDING);
        
        if (size >= 0) {
            
            char* aEncryptedResponse;
            long aLen = base64_encode(encryptedResponse, size, &aEncryptedResponse);

            if (response != NULL)
                memcpy(response, aEncryptedResponse, aLen);
            if (resLength != NULL)
                *resLength = aLen;
            
            free(aEncryptedResponse);
            
        } else {
            log_message(LOG_ERROR, "Unable to encrypt Apple response");
            if (resLength != NULL)
                *resLength = 0;
        }
        
    } else {
        log_message(LOG_ERROR, "Could not find interface for IP");
        if (resLength != NULL)
            *resLength = 0;
    }
        
}

bool RAOPConnection::_checkAuthentication(const char* method, const char* uri, const char* authenticationParameter) {
    
    assert(method != NULL && uri != NULL);
    
    if (_password != NULL) {
        
        if (authenticationParameter != NULL) {
            
            const char* paramBegin = strstr(authenticationParameter, " ") + 1;
            if (paramBegin) {
                RAOPParameters* parameters = new RAOPParameters(paramBegin, strlen(paramBegin), ParameterTypeDigest);
                
                const char* nonce = parameters->valueForParameter("nonce");
                const char* response = parameters->valueForParameter("response");
                
                char wResponse[strlen(response) + 1];
                strcpy(wResponse, response);
                
                // Check if nonce is correct
                if (nonce != NULL && strlen(nonce) == 32 && strcmp(nonce, _digestNonce) == 0) {
                    
                    const char* username = parameters->valueForParameter("username");
                    const char* realm = parameters->valueForParameter("realm");
                    int pwLen = strlen(_password);
                    
                    char a1pre[strlen(username) + strlen(realm) + pwLen + 3];
                    sprintf(a1pre, "%s:%s:%s", username, realm, _password);
                    
                    char a2pre[strlen(method) + strlen(uri) + 2];
                    sprintf(a2pre, "%s:%s", method, uri);
                    
                    unsigned char a1[16], a2[16];
                    md5(a1pre, strlen(a1pre), a1);
                    md5(a2pre, strlen(a2pre), a2);
                    
                    char ha1[33], ha2[33];
                    md5ToString(a1, ha1);
                    md5ToString(a2, ha2);
                    
                    char finalpre[67 + strlen(_digestNonce)];
                    sprintf(finalpre, "%s:%s:%s", ha1, _digestNonce, ha2);
                    
                    unsigned char final[16];
                    md5(finalpre, strlen(finalpre), final);
                    
                    char hfinal[33];
                    md5ToString(final, hfinal);
                    
                    for (int i = 0 ; i < 32 ; i++) {
                        hfinal[i] = tolower(hfinal[i]);
                        wResponse[i] = tolower(wResponse[i]);
                    }
                    
                    if (strcmp(hfinal, wResponse) == 0) {
                        log_message(LOG_INFO, "Authentication success");
                        return true;
                    }
                    
                }
                
            }
            
        } else
            log_message(LOG_INFO, "Authentication header missing");
        
        return false;
        
    }
    
    return true;
    
}

RAOPConnectionAudioRoute RAOPConnection::_getAudioRoute() {
    
#if TARGET_OS_IPHONE
    CFStringRef route = NULL;
    UInt32 size = sizeof(CFStringRef);
    AudioSessionGetProperty(kAudioSessionProperty_AudioRoute, &size, &route);
    
    int length = CFStringGetLength(route);
    if (route != NULL && length > 0) {
        
        char cRoute[length+1];
        if (CFStringGetCString(route, cRoute, length+1, kCFStringEncodingASCII)) {
            log_message(LOG_INFO, "Audio route: %s", cRoute);
            if (strcmp(cRoute, "Headphone") == 0)
                return kRAOPConnectionAudio_RouteHeadphones;
            else if (strcmp(cRoute, "AirTunes") == 0)
                return kRAOPConnectionAudio_RouteAirPlay;            
        }
        
    }
#endif
    
    return kRAOPConnectionAudio_RouteUnknown;
    
}

void RAOPConnection::_processRequestCallback(web_connection_p connection, web_request_p request) {
    
    int cSeq = 0;    
    
    const char* cmd = web_request_get_command(request);
    const char* path = web_request_get_path(request);
    web_response_p response = web_request_get_response(request);
    web_headers_p requestHeaders = web_request_get_headers(request);
    web_headers_p responseHeaders = web_response_get_headers(response);
    
    web_response_set_status(response, 200, "OK");
    web_response_set_keep_alive(response, true);
    
    if (cmd != NULL && path != NULL) {
    
        log_message(LOG_INFO, "CMD: %s / PATH: %s", cmd, path);
        
        RAOPParameters* parameters = NULL;
        
        const char* cSeqValue = web_headers_value(requestHeaders, "CSeq");
        if (cSeqValue != NULL)
            cSeq = atoi(cSeqValue);
        
        uint32_t contentLength;
        if ((contentLength = web_request_get_content(request, NULL, 0)) > 0) {
            const char* contentType = web_headers_value(requestHeaders, "Content-Type");
            if (strcmp(contentType, "application/sdp") == 0 || strcmp(contentType, "text/parameters") == 0) {
                
                char* content[contentLength];
                web_request_get_content(request, content, contentLength);
                
                int cl = web_tools_convert_new_lines((unsigned char*)content, contentLength);
                if (strcmp(contentType, "application/sdp") == 0)
                    parameters = new RAOPParameters((char*)content, cl, ParameterTypeSDP);
                else if (strcmp(contentType, "text/parameters") == 0)
                    parameters = new RAOPParameters((char*)content, cl, ParameterTypeTextParameters);
                
                log_data(LOG_INFO, (char*)content, contentLength);
                
            }
        }
        
        const char* dacpId;
        if (_dacpId[0] == '\0' && (dacpId = web_headers_value(requestHeaders, "DACP-ID")) != NULL) {
            memset(_dacpId, '0', 16);
            int32_t start = 16 - strlen(dacpId);
            if (start >= 0 && start < DACPID_MAXLENGTH)
                strncpy(&_dacpId[start], dacpId, DACPID_MAXLENGTH);
        }
        
        const char* activeRemote;
        if (_activeRemote[0] == '\0' && (activeRemote = web_headers_value(requestHeaders, "Active-Remote")) != NULL)
            strncpy(_activeRemote, activeRemote, ACTIVEREMOTE_MAXLENGTH);
        
        const char *sessionId;
        if (_sessionId[0] == '\0' && (sessionId = web_headers_value(requestHeaders, "Session")) != NULL)
            strncpy(_sessionId, sessionId, SESSION_MAXLENGTH);
        
        const char *userAgent;
        if (_userAgent[0] == '\0' && (userAgent = web_headers_value(requestHeaders, "User-Agent")) != NULL)
            strncpy(_userAgent, userAgent, USERAGENT_MAXLENGTH);
        
        web_headers_set_value(responseHeaders, "Server", "AirTunes/105.1");
        web_headers_set_value(responseHeaders, "CSeq", "%d", cSeq);
        
        if (_checkAuthentication(cmd, path, web_headers_value(requestHeaders, "Authorization"))) {
            
            if (0 == strcmp(cmd, "OPTIONS"))
                web_headers_set_value(responseHeaders, "Public", "ANNOUNCE, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, GET_PARAMETER, SET_PARAMETER, POST, GET");
            else if (0 == strcmp(cmd, "ANNOUNCE")) {
                
                const char* codec = NULL;
                int codecIdentifier;
                
                const char* rtpmap;
                if ((rtpmap = parameters->valueForParameter("a-rtpmap")) != NULL) {
                    
                    codecIdentifier = atoi(rtpmap);
                    for (long i = 0 ; i < strlen(rtpmap) - 1 ; i++)
                        if (rtpmap[i] == ' ') {
                            codec = &rtpmap[i+1];
                            break;
                        }
                    
                    const char* fmtp;
                    if ((fmtp = parameters->valueForParameter("a-fmtp")) != NULL && atoi(fmtp) == codecIdentifier) {
                        
                        int fmtpSize = 0;
                        size_t fmtpLen = strlen(fmtp);
                        for (long i = 0 ; i < fmtpLen ; i++)
                            if (fmtp[i] == ' ')
                                fmtpSize++;
                        
                        int fmtps[fmtpSize];
                        int cfmtp = 0;
                        for (long i = 0 ; i < fmtpLen ; i++)
                            if (fmtp[i] == ' ')
                                fmtps[cfmtp++] = atoi(&fmtp[i +1]);
                        
                        unsigned char aeskey[16];
                        unsigned char aesiv[16];
                        
                        bzero(aeskey, 16);
                        bzero(aesiv, 16);
                        
                        if (_rtp != NULL) {
                            delete _rtp;
                            _rtp = NULL;
                        }
                        
                        const char* aesKeyBase64Encoded;
                        if ((aesKeyBase64Encoded = parameters->valueForParameter("a-rsaaeskey")) != NULL) {
                            
                            char aesKeyBase64EncodedPadded[strlen(aesKeyBase64Encoded) + 5];
                            long size = padBase64(aesKeyBase64Encoded, strlen(aesKeyBase64Encoded), aesKeyBase64EncodedPadded);
                            char aesKeyEncryptet[size];
                            size = base64_decode(aesKeyBase64EncodedPadded, aesKeyEncryptet);
                            
                            unsigned char aesKey[size + 1];
                            size = RSA_private_decrypt((int)size, (unsigned char*)aesKeyEncryptet, aesKey, _applePrivateKey, RSA_PKCS1_OAEP_PADDING);
                            
                            if (size >= 16)
                                memcpy(aeskey, aesKey, 16);
                            
                            log_message(LOG_INFO, "AES key length: %d bits", size * 8);
                            
                            const char* aesInitializerBase64Encoded = parameters->valueForParameter("a-aesiv");
                            char aesInitializerBase64EncodedPadded[strlen(aesInitializerBase64Encoded) + 5];
                            size = padBase64(aesInitializerBase64Encoded, strlen(aesInitializerBase64Encoded), aesInitializerBase64EncodedPadded);
                            char aesInitializer[size];
                            size = base64_decode(aesInitializerBase64EncodedPadded, aesInitializer);
                            
                            if (size >= 16)
                                memcpy(aesiv, aesInitializer, 16);
                            
                            log_message(LOG_INFO, "AES initialization vector length: %d bits", size * 8);
                            
                            _rtp = new RTPReceiver(aeskey, aesiv, this, fmtps, fmtpSize);
                            
                        } else // Stream is clear text
                            _rtp = new RTPReceiver(this, fmtps, fmtpSize);
                        
                    } else
                        web_response_set_status(response, 400, "Bad Request");
                    
                } else
                    web_response_set_status(response, 400, "Bad Request");
                
            } else if (0 == strcmp(cmd, "SETUP")) {
                
                if (RTPReceiver::isAvailable() && _getAudioRoute() != kRAOPConnectionAudio_RouteAirPlay) {
                    
                    const char* transport;
                    if (_rtp != NULL && (transport = web_headers_value(requestHeaders, "Transport"))) {
                        
                        char session[10];
                        _rtp->getSession(session);
                        web_headers_set_value(responseHeaders, "Session", "%02X%02X%02X%02X", session[0], session[1], session[2], session[3]);
                        
                        RAOPParameters* transportParams = new RAOPParameters(transport, strlen(transport), ParameterTypeSDP, ';');
                        
                        unsigned short timing_port, control_port;
                        timing_port = control_port = 0;
                        
                        const char *tp, *cp;
                        if ((tp = transportParams->valueForParameter("timing_port"))) {
                            timing_port = atoi(tp);
                            transportParams->removeParameter("timing_port");
                        }
                        if ((cp = transportParams->valueForParameter("control_port"))) {
                            control_port = atoi(cp);
                            transportParams->removeParameter("control_port");
                        }
                        
                        unsigned short port;
                        if ((port = _rtp->setup(&timing_port, &control_port))) {
                            
                            char* transportReply = (char*)malloc(1);
                            transportReply[0] = '\0';
                            
                            for (long i = 0 ; i < transportParams->count() ; i++) {
                                const char* key = transportParams->parameterAtIndex(i);
                                const char* value;
                                if ((value = transportParams->valueForParameter(key)) != key)
                                    transportReply = addToBuffer(transportReply, "%s=%s;", key, value);
                                else
                                    transportReply = addToBuffer(transportReply, "%s;", key);
                            }
                            
                            if (control_port > 0)
                                transportReply = addToBuffer(transportReply, "control_port=%d;", control_port);
                            if (timing_port > 0)
                                transportReply = addToBuffer(transportReply, "timing_port=%d;", timing_port);
                            
                            transportReply = addToBuffer(transportReply, "server_port=%d", port);
                            
                            web_headers_set_value(responseHeaders, "Transport", transportReply);
                            
                            free(transportReply);
                            
                            log_message(LOG_INFO, "RTP receiver set up at port %d", port);
                            
                            notification_center_post_notification(RAOPConnection::clientConnectedNotificationName, this, NULL);
                            
                        } else
                            log_message(LOG_ERROR, "RTP Receiver didn't start");
                        
                    } else                            
                        web_response_set_status(response, 400, "Bad Request");
                    
                } else
                    web_response_set_status(response, 453, "Not Enough Bandwidth");
                
            } else if (0 == strcmp(cmd, "RECORD")) {
                
                char session[10];
                _rtp->getSession(session);
                web_headers_set_value(responseHeaders, "Session", "%02X%02X%02X%02X", session[0], session[1], session[2], session[3]);
                
                const char* rtpinfo;
                if ((rtpinfo = web_headers_value(requestHeaders, "RTP-Info"))) {
                    
                    RAOPParameters* rtpParams = new RAOPParameters(rtpinfo, strlen(rtpinfo), ParameterTypeSDP, ';');
                    if (!_rtp->start(atoi(rtpParams->valueForParameter("seq"))))
                        web_response_set_status(response, 400, "Bad Request");
                    
                    delete rtpParams;
                    
                } else {
                    _rtp->start();
                }
                
                notification_center_post_notification(RAOPConnection::recordingStartedNotificationName, this, NULL);
                
                web_headers_set_value(responseHeaders, "Audio-Latency", "11025");
                
            } else if (0 == strcmp(cmd, "SET_PARAMETER")) {
                
                if (parameters != NULL) {
                    
                    const char* volume;
                    const char* progress;
                    if ((volume = parameters->valueForParameter("volume"))) {
                        float fVolume;
                        sscanf(volume, "%f", &fVolume);
                        log_message(LOG_INFO, "Client set volume: %f", (30.0 + fVolume) / 30.0);
                        
                        fVolume = (fVolume == -144 ? 0.0 : (30.0 + fVolume) / 30.0);
                        _rtp->getAudioPlayer()->setVolume(fVolume * fVolume * fVolume);
                        
                    } else if ((progress = parameters->valueForParameter("progress"))) {
                        
                        const char* parts[3] = { NULL, NULL, NULL };
                        int cPart = 1;
                        parts[0] = progress;
                        
                        for (int i = 0 ; i < strlen(progress) ; i++)
                            if (progress[i] == '/')
                                parts[cPart++] = &progress[i+1];
                        
                        double start = atoi(parts[0]);
                        
                        RAOPConnectionClientUpdatedProgressNotificationInfo info;
                        info.position = (atoi(parts[1]) - start) / _rtp->getAudioPlayer()->getSampleRate();
                        info.total = (atoi(parts[2]) - start) / _rtp->getAudioPlayer()->getSampleRate();
                        
                        notification_center_post_notification(RAOPConnection::clientUpdatedProgressNotificationName, this, &info);
                        
                    }
                    
                } else {
                    
                    const char* contentType = web_headers_value(requestHeaders, "Content-Type");
                    
                    uint32_t contentLength = web_request_get_content(request, NULL, 0);
                    char* content = (char*)malloc(contentLength);
                    web_request_get_content(request, content, contentLength);
                    
                    if (0 == strcmp(contentType, "application/x-dmap-tagged")) {
                        
                        dmap_p tags = dmap_create();
                        dmap_parse(tags, content, contentLength);
                        
                        uint32_t containerTag;
                        if (dmap_get_count(tags) > 0 && dmap_type_for_tag((containerTag = dmap_get_tag_at_index(tags, 0))) == dmap_type_container) {
                            
                            dmap_p trackTags = dmap_container_for_atom_tag(tags, containerTag);
                            notification_center_post_notification(RAOPConnection::clientUpdatedTrackInfoNofificationName, this, trackTags);
                            
                            const char* title = dmap_string_for_atom_identifer(trackTags, "dmap.itemname");
                            const char* artist = dmap_string_for_atom_identifer(trackTags, "dmap.songartist");
                            if (title != NULL && artist != NULL)
                                log_message(LOG_INFO, "Now playing: \"%s\" by %s", title, artist);
                            
                        }
                        
                        dmap_destroy(tags);
                        
                    } else {
                    
                        log_message(LOG_INFO, "Track info received (Content-Type: %s)", contentType);
                        RAOPConnectionClientUpdatedMetadataNotificationInfo info;
                        info.content = content;
                        info.contentLength = contentLength;
                        info.contentType = contentType;
                        
                        notification_center_post_notification(RAOPConnection::clientUpdatedMetadataNotificationName, this, &info);

                    }
                    
                    free(content);
                    
                }
                
            } else if (0 == strcmp(cmd, "FLUSH")) {
                
                int32_t lastSeq = -1;
                const char* rtpInfo;
                if ((rtpInfo = web_headers_value(requestHeaders, "RTP-Info")) != NULL) {
                    
                    RAOPParameters* rtpParams = new RAOPParameters(rtpInfo, strlen(rtpInfo), ParameterTypeSDP, ';');
                    
                    const char* seq;
                    if ((seq = rtpParams->valueForParameter("seq")) != NULL)
                        lastSeq = atoi(seq);
                }
                
                _rtp->getAudioPlayer()->flush(lastSeq);
                
            } else if (0 == strcmp(cmd, "TEARDOWN")) {
                
                web_headers_set_value(responseHeaders, "Connection", "Close");
                web_response_set_keep_alive(response, false);
                
                notification_center_post_notification(RAOPConnection::clientDisconnectedNotificationName, this, NULL);
                
            } else
                web_response_set_status(response, 400, "Bad Request");
            
        } else {
            
            unsigned char nonce[16];
            
            for (int i = 0 ; i < 16 ; i++)
                nonce[i] = (char) rand() % 256;
            
            md5ToString(nonce, _digestNonce);
            
            web_headers_set_value(responseHeaders, "WWW-Authenticate", "Digest realm=\"raop\", nonce=\"%s\"", _digestNonce);
            
            web_response_set_status(response, 401, "Unauthorized");
            
        }
        
        const char* challenge;
        if ((challenge = web_headers_value(requestHeaders, "Apple-Challenge"))) {
            
            char ares[1000];
            long resLength = 100;
            
            char rchallenge[strlen(challenge) + 5];
            padBase64(challenge, strlen(challenge), rchallenge);
            _appleResponse(rchallenge, strlen(rchallenge), ares, &resLength);
            
            if (resLength > 0) {
                ares[resLength] = '\0';
                web_headers_set_value(responseHeaders, "Apple-Response", "%s", ares);
            }
            
        }
        
        web_headers_set_value(responseHeaders, "Audio-Jack-Status", "connected; type=digital");
                    
        if (parameters != NULL)
            delete parameters;
        
    } else
        web_response_set_status(response, 400, "Bad Request");
    
}

void RAOPConnection::_processRequestCallbackHelper(web_connection_p connection, web_request_p request, void* ctx) {
    
    ((RAOPConnection*)ctx)->_processRequestCallback(connection, request);
    
}

void RAOPConnection::_connectionClosedCallback(web_connection_p connection) {
    
    delete this;

}

void RAOPConnection::_connectionClosedCallbackHelper(web_connection_p connection, void* ctx) {
    
    ((RAOPConnection*)ctx)->_connectionClosedCallback(connection);
    
}

const char* RAOPConnection::getDacpId() {
    
    return _dacpId;
    
}

const char* RAOPConnection::getActiveRemote() {
    
    return _activeRemote;
    
}

const char* RAOPConnection::getSessionId() {
    
    return _sessionId;
    
}

const char* RAOPConnection::getUserAgent() {
    
    return _userAgent;
    
}

bool RAOPConnection::isConnected() {
    
    return web_connection_is_connected(_connection);
    
}

void RAOPConnection::closeConnection() {
    
    if (_connection != NULL)
        web_connection_close(_connection);
    
}

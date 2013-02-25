//
//  RAOPConnection.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __RAOPCONNECTION_H
#define __RAOPCONNECTION_H

#include <openssl/rsa.h>

#include <pthread.h>
#include "RTPReceiver.h"

extern "C" {
#include "socket.h"
#include "sockaddr.h"
#include "webtools.h"
#include "webrequest.h"
#include "webconnection.h"
}

#define DACPID_MAXLENGTH 200
#define ACTIVEREMOTE_MAXLENGTH 50
#define SESSION_MAXLENGTH 50
#define USERAGENT_MAXLENGTH 256

typedef enum {
    kRAOPConnectionAudio_RouteAirPlay = 0,
    kRAOPConnectionAudio_RouteHeadphones,
    kRAOPConnectionAudio_RouteUnknown
} RAOPConnectionAudioRoute;

typedef struct {
    const void* content;
    uint32_t contentLength;
    const char* contentType;
} RAOPConnectionClientUpdatedMetadataNotificationInfo;

typedef struct {
    double position;
    double total;
} RAOPConnectionClientUpdatedProgressNotificationInfo;

class WebConnection;
class WebRequest;
class RAOPConnection;

class RAOPConnection {

    friend class RAOPServer;
    
public:
    RAOPConnection(web_connection_p connection);
    ~RAOPConnection();
    
    RTPReceiver* getRTPReceiver();
    
    struct sockaddr* getLocalEndPoint();
    struct sockaddr* getRemoteEndPoint();
    
    const char* getDacpId();
    const char* getActiveRemote();
    const char* getSessionId();
    
    const char* getUserAgent();
    
    bool isConnected();
    
    void closeConnection();
    
    static const char* recordingStartedNotificationName;
    static const char* clientConnectedNotificationName;
    static const char* clientDisconnectedNotificationName;
    static const char* clientUpdatedTrackInfoNofificationName;
    static const char* clientUpdatedMetadataNotificationName;
    static const char* clientUpdatedProgressNotificationName;
    
private:
    void _appleResponse(const char* challenge, long length, char* response, long* resLength);
    bool _checkAuthentication(const char* method, const char* uri, const char* authenticationParameter);
    
    void _processRequestCallback(web_connection_p connection, web_request_p request);
    static void _processRequestCallbackHelper(web_connection_p connection, web_request_p request, void* ctx);
    void _connectionClosedCallback(web_connection_p connection);
    static void _connectionClosedCallbackHelper(web_connection_p connection, void* ctx);
    
    RAOPConnectionAudioRoute _getAudioRoute();
    
    web_connection_p _connection;
    
    RSA* _applePrivateKey;
    
    RTPReceiver* _rtp;
    
    bool _authenticationEnabled;
    char* _password;
    char _digestNonce[33];
        
    char _dacpId[DACPID_MAXLENGTH];
    char _activeRemote[ACTIVEREMOTE_MAXLENGTH];
    char _sessionId[SESSION_MAXLENGTH];
    
    char _userAgent[USERAGENT_MAXLENGTH];
    
};

#endif

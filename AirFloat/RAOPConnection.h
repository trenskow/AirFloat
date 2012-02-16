//
//  RAOPConnection.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __RAOPCONNECTION_H
#define __RAOPCONNECTION_H

#include <CoreFoundation/CoreFoundation.h>
#include <pthread.h>
#include "RTPReceiver.h"
#include "Socket.h"

#define DACPID_MAXLENGTH 200
#define ACTIVEREMOTE_MAXLENGTH 50
#define SESSION_MAXLENGTH 50

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

#include "WebTools.h"

class WebConnection;
class WebRequest;
class RAOPConnection;

class RAOPConnection : public WebTools {

    friend class RAOPServer;
    
public:
    RAOPConnection(WebConnection* connection);
    ~RAOPConnection();
    
    RTPReceiver* getRTPReceiver();
    
    SocketEndPoint getRemoteEndPoint();
    
    const char* getDacpId();
    const char* getActiveRemote();
    const char* getSessionId();
    
    unsigned int getNetworkScopeId();
    
    static const char* recordingStartedNotificationName;
    static const char* clientDisconnectedNotificationName;
    static const char* clientUpdatedTrackInfoNofificationName;
    static const char* clientUpdatedMetadataNotificationName;
    static const char* clientUpdatedProgressNotificationName;
    
private:
    void _appleResponse(const char* challenge, long length, char* response, long* resLength);
    bool _checkAuthentication(const char* method, const char* uri, const char* authenticationParameter);
    
    void _processRequestCallback(WebConnection* connection, WebRequest* request);
    static void _processRequestCallbackHelper(WebConnection* connection, WebRequest* request, void* ctx);
    void _connectionClosedCallback(WebConnection* connection);
    static void _connectionClosedCallbackHelper(WebConnection* connection, void* ctx);
    
    RAOPConnectionAudioRoute _getAudioRoute();
    
    WebConnection* _connection;
    
    RTPReceiver* _rtp;
    
    bool _authenticationEnabled;
    CFStringRef _password;
    char _digestNonce[33];
        
    char _dacpId[DACPID_MAXLENGTH];
    char _activeRemote[ACTIVEREMOTE_MAXLENGTH];
    char _sessionId[SESSION_MAXLENGTH];
    
};

#endif

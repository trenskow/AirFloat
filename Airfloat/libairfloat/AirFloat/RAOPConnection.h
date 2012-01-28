//
//  RAOPConnection.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __RAOPCONNECTION_H
#define __RAOPCONNECTION_H

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

class RAOPConnection;

typedef void(*simpleConnectionClbk)(RAOPConnection* connection, void* ctx);
typedef void(*clientSetVolumeClbk)(RAOPConnection* connection, float volume, void* ctx);
typedef void(*clientUpdatedMetadataClbk)(RAOPConnection* connection, void* buffer, int size, const char* contentType, void* ctx);

class RAOPConnection {

    friend class RAOPServer;
    
public:
    RAOPConnection(Socket* sock);
    ~RAOPConnection();
    
    RTPReceiver* GetRTPReceiver();
    
    SocketEndPoint GetHost();
    
    void SetCallbacksContext(void* ctx);
    void SetRecordingStartedClbk(simpleConnectionClbk clbk);
    void SetRecordingStoppedClbk(simpleConnectionClbk clbk);
    void SetClientDisconnectedClbk(simpleConnectionClbk clbk);
    void SetClientSetVolumeClbk(clientSetVolumeClbk clbk);
    void SetClientUpdatedMetadataClbk(clientUpdatedMetadataClbk clbk);
    
    const char* GetDacpId();
    const char* GetActiveRemote();
    const char* GetSessionId();
    
    unsigned int GetNetworkScopeId();
    
private:
    void _takeOver();
    void _appleResponse(const char* challenge, long length, char* response, long* resLength);
    bool _checkAuthentication(const char* method, const char* uri, const char* authenticationParameter);
    bool _processData(unsigned char* buffer, long length);
    static void* _connectionLoopKickStarter(void* t);
    void _connectionLoop();
    RAOPConnectionAudioRoute _getAudioRoute();
    
    long _ensureNewLines(unsigned char* buffer, long length);
    pthread_t _connectionLoopThread;
    
    Socket* _sock;
    
    RTPReceiver* _rtp;
    
    char _digestNonce[33];
    
    void* _clbkCtx;
    simpleConnectionClbk _recordingStartedClbk;
    simpleConnectionClbk _recordingStoppedClbk;
    simpleConnectionClbk _clientDisconnectedClbk;
    clientSetVolumeClbk _clientSetVolumeClbk;
    clientUpdatedMetadataClbk _clientUpdatedMetadataClbk;
    
    char _dacpId[DACPID_MAXLENGTH];
    char _activeRemote[ACTIVEREMOTE_MAXLENGTH];
    char _sessionId[SESSION_MAXLENGTH];
    
};

#endif

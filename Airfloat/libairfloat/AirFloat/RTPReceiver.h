//
//  RTPReceiver.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __RTPRECEIVER_H
#define __RTPRECEIVER_H

#include <openssl/aes.h>

#define SYNC_BACKLOG 10

#include "AudioPlayer.h"
#include "Socket.h"

typedef struct {
    
    unsigned short seq_num;
    
    bool extension;
    unsigned char source;
    unsigned char payload_type;
    bool marker;
    
    unsigned char* packet_data;
    int packet_data_length;
    
} RTPPacket;

class RAOPConnection;

class RTPReceiver {
    
public:
    RTPReceiver(const char* localHost, const char* remoteHost, const unsigned char aesKey[16], const unsigned char aesIv[16], RAOPConnection* connection, int* fmtp, int fmtpLen);
    RTPReceiver(const char* localHost, const char* remoteHost, RAOPConnection* connection, int* fmtp, int fmtpLen);
    ~RTPReceiver();
    
    unsigned short setup(unsigned short* timingPort, unsigned short* controlPort);
    void start(unsigned short startSeq = 0);
    void stop();
    bool started();
    
    int GetSession(char* to);
    static bool IsAvailable();
    
    AudioPlayer* getAudioPlayer();
    
private:
    
    void _setup(const char* localHost, const char* remoteHost, RAOPConnection* connection, int* fmtp, int fmtpLen);
    
    void _processSyncPacket(RTPPacket* packet);
    void _processTimingResponsePacket(RTPPacket* packet);
    void _sendPacket(const char* buffer, size_t len, unsigned short port, Socket* sock);
    void _sendTimingRequest();
    void _sendResendRequest(unsigned short seqNum, unsigned short count);
    void _startSynchronizationLoop();
    void _stopSynchronizationLoop();
    
    static void _queueFlushCalback(AudioQueue* queue, void* ctx);
    static void _queueSyncCalback(AudioQueue* queue, void* ctx);
    
    void _synchronizationLoop();
    static void* _synchronizationLoopKickStarter(void* t);
    
    void _processAudioPacket(RTPPacket* packet);
    
    static void* _streamLoopKickStarter(void* t);
    void _streamLoop(Socket* sock);
    pthread_t _start(Socket* sock, const char* name);
    
    Socket* _setupSocket(unsigned short* port);
    
    RAOPConnection* _connection;
    
    pthread_t _serverSockThread, _timingSockThread, _controlSockThread;
    
    pthread_mutex_t _timerMutex;
    pthread_cond_t _timerCond;
    pthread_cond_t _syncCond;
    pthread_t _timerThread;
    
    bool _timerRunning;
    int _timeResponseCount;
    
    Socket* _serverSock;
    Socket* _timingSock;
    Socket* _controlSock;
    
    char* _localHost;
    char* _remoteHost;
    
    unsigned short _clientTimingPort;
    unsigned short _clientControlPort;
    
    bool _streamIsEncrypted;
    AES_KEY _aes;
    unsigned char _aesKey[16];
    unsigned char _aesIv[16];
    
    unsigned char _session[4];
    
    AudioPlayer* _audioPlayer;
    
};

#endif

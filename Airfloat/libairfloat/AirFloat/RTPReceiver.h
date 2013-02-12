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

#include "NotificationCenter.h"
#include "Mutex.h"
#include "Condition.h"
#include "AudioPlayer.h"
#include "Socket.h"

typedef struct {
    
    uint16_t seq_num;
    
    bool extension;
    uint8_t source;
    uint8_t payload_type;
    bool marker;
    
    uint8_t* packet_data;
    uint32_t packet_data_length;
    
} RTPPacket;

class RAOPConnection;

class RTPReceiver : public NotificationObserver {
    
public:
    RTPReceiver(const unsigned char aesKey[16], const unsigned char aesIv[16], RAOPConnection* connection, int* fmtp, int fmtpLen);
    RTPReceiver(RAOPConnection* connection, int* fmtp, int fmtpLen);
    ~RTPReceiver();
    
    unsigned short setup(unsigned short* timingPort, unsigned short* controlPort);
    bool start(unsigned short startSeq = 0);
    void stop();
    bool started();
    
    int getSession(char* to);
    static bool isAvailable();
    static RTPReceiver* getStreamingReceiver();
    
    RAOPConnection* getConnection();
    AudioPlayer* getAudioPlayer();
    
    void _notificationReceived(Notification* notification);
    
private:
    
    void _setup(RAOPConnection* connection, int* fmtp, int fmtpLen);
    
    void _processSyncPacket(RTPPacket* packet);
    void _processTimingResponsePacket(RTPPacket* packet);
    void _sendPacket(const char* buffer, size_t len, SocketEndPoint* remoteEndPoint, Socket* sock);
    void _sendTimingRequest();
    void _sendResendRequest(unsigned short seqNum, unsigned short count);
    void _startSynchronizationLoop();
    void _stopSynchronizationLoop();
    
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
    
    Mutex _timerMutex;
    Condition _timerCond;
    Condition _syncCond;
    pthread_t _timerThread;
    
    bool _timerRunning;
    int _timeResponseCount;
    
    Socket* _serverSock;
    Socket* _timingSock;
    Socket* _controlSock;
    
    SocketEndPoint* _localEndPoint;
    SocketEndPoint* _remoteTimingEndPoint;
    SocketEndPoint* _remoteControlEndPoint;
    
    bool _streamIsEncrypted;
    AES_KEY _aes;
    unsigned char _aesKey[16];
    unsigned char _aesIv[16];
    
    unsigned char _session[4];
    
    AudioPlayer* _audioPlayer;
    
};

#endif

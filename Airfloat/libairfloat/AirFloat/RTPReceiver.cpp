//
//  RTPReceiver.cpp
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#include <QuartzCore/QuartzCore.h>

#include "Mutex.h"
#include "Log.h"
#include "RAOPConnection.h"
#include "RTPReceiver.h"

#define RTP_EXTENSION                   0x10
#define RTP_SOURCE                      0x0f
#define RTP_PAYLOAD_TYPE                0x7f
#define RTP_MARKER                      0x80

#define RTP_TIMING_REQUEST              0x52
#define RTP_TIMING_RESPONSE             0x53
#define RTP_SYNC                        0x54
#define RTP_RANGE_RESEND_REQUEST        0x55
#define RTP_AUDIO_RESEND_DATA           0x56
#define RTP_AUDIO_DATA                  0x60

#define SafeDR(x, v) if (x != NULL) *x = v
#ifndef MIN
#define MIN(x, y) (x < y ? x : y)
#endif

static Mutex __globalMutex;
static RTPReceiver* __globalReceiver = NULL;

typedef struct {
    
    uint32_t integer;
    uint32_t fraction;
    
} NTPTime;

typedef struct {
    
    RTPReceiver* t;
    Socket* sock;
    char name[255];
    
} _streamLoopKickStarterHelper;

typedef struct {
    
    uint8_t a;
    uint8_t b;
    uint16_t seq_num;
    
    uint32_t _padding;
    
    NTPTime reference_time;
    NTPTime received_time;
    NTPTime send_time;
    
} RTPTimingPacket;

#define RTPTimingPacketSize 32

typedef struct {
    
    uint8_t a;
    uint8_t b;
    uint16_t seq_num; // unused
    
    uint16_t missed_seq;
    uint16_t count;
    
} RTPResendPacket;

#define RTPResendPacketSize 8

const uint32_t NTP_UNIXEPOCH = 0x83aa7e80;
const double NTP_FRACTION = 0xFFFFFFFF;

double ntpTimeToDouble(NTPTime time) {
    
    uint32_t i = ntohl(time.integer);
    i -= NTP_UNIXEPOCH;
    return i + (ntohl(time.fraction) / NTP_FRACTION);
    
}

NTPTime doubleToNtpTime(double time) {
    
    NTPTime ret;
    
    double integer = floor(time);
    uint32_t i = integer;
    i += NTP_UNIXEPOCH;
    ret.integer = htonl(i);
    ret.fraction = (time - integer) * NTP_FRACTION;
    
    ret.fraction = htonl(ret.fraction);
    
    return ret;
    
}

RTPPacket readHeader(const unsigned char* buffer, long length) {
    
    assert(buffer != NULL && length > 0);
    
    RTPPacket ret;
    memset(&ret, 0, sizeof(RTPPacket));
    
    unsigned char a = *((unsigned char*)&buffer[0]);
    unsigned char b = *((unsigned char*)&buffer[1]);
    
    ret.seq_num = ntohs(*((unsigned short*)&buffer[2]));
    
    ret.extension = (bool)(a & RTP_EXTENSION);
    ret.source = a & RTP_SOURCE;
    ret.payload_type = b & RTP_PAYLOAD_TYPE;
    ret.marker = b & RTP_MARKER;
    
    ret.packet_data = (unsigned char*) &buffer[4];
    ret.packet_data_length = length - 4;
    
    return ret;
    
}

RTPReceiver::RTPReceiver(const unsigned char aesKey[16], const unsigned char aesIv[16], RAOPConnection* connection, int* fmtp, int fmtpLen) {
    
    assert(connection != NULL && fmtp != NULL);
    
    _setup(connection, fmtp, fmtpLen);
    
    memcpy(_aesKey, aesKey, 16);
    memcpy(_aesIv, aesIv, 16);
    
    memset(&_aes, 0, sizeof(AES_KEY));
    AES_set_decrypt_key(_aesKey, 128, &_aes);
    
    _streamIsEncrypted = true;
    
}

RTPReceiver::RTPReceiver(RAOPConnection* connection, int* fmtp, int fmtpLen) {
    
    _setup(connection, fmtp, fmtpLen);
    
}

RTPReceiver::~RTPReceiver() {
    
    delete _audioPlayer;

    _connection = NULL;
    stop();    
    
}

void RTPReceiver::_setup(RAOPConnection* connection, int* fmtp, int fmtpLen) {
    
    assert(connection != NULL && fmtp != NULL);

    _serverSock = _timingSock = _controlSock = 0;
    _remoteTimingEndPoint = _remoteControlEndPoint = NULL;
    
    for (int i = 0 ; i < 4 ; i++)
        _session[i] = (char)rand();
    
    _connection = connection;
    
    _streamIsEncrypted = false;
    
    _timerThread = _serverSockThread = _timingSockThread = _controlSockThread = NULL;
    
    _timeResponseCount = 0;
    
    _audioPlayer = new AudioPlayer(fmtp, fmtpLen);
    
    _timerRunning = false;
    
    NotificationCenter::defaultCenter()->addObserver(this, AudioQueue::flushNotificationName, NULL);
    NotificationCenter::defaultCenter()->addObserver(this, AudioQueue::syncNotificationName, NULL);
    
}

int RTPReceiver::getSession(char* to) {
    
    assert(to != NULL);
    
    sprintf(to, "%02X%02X%02X%02X", _session[0], _session[1], _session[2], _session[3]);
    return 9;
    
}

bool RTPReceiver::started() {
    
    return (_timerThread != NULL);
    
}

bool RTPReceiver::isAvailable() {
    
    __globalMutex.lock();
    bool ret = !(__globalReceiver != NULL && __globalReceiver->started());
    __globalMutex.unlock();
    
    return ret;
    
}

RTPReceiver* RTPReceiver::getStreamingReceiver() {
    
    __globalMutex.lock();
    RTPReceiver* ret = __globalReceiver;
    __globalMutex.unlock();
    
    return ret;
    
}

RAOPConnection* RTPReceiver::getConnection() {
    
    return _connection;
    
}

AudioPlayer* RTPReceiver::getAudioPlayer() {
    
    return _audioPlayer;
    
}

void RTPReceiver::_processSyncPacket(RTPPacket* packet) {
    
    assert(packet != NULL);
    
    uint32_t currentRTPTime = ntohl(*(uint32_t*)&packet->packet_data[0]);
    double currentTime = ntpTimeToDouble(*((NTPTime*)&packet->packet_data[4]));
    uint32_t nextRTPTime = ntohl(*(uint32_t*)&packet->packet_data[12]);
    
    log(LOG_INFO, "Sync packet (Playhead frame: %u - current time: %1.6f - next frame: %u)", currentRTPTime, currentTime, nextRTPTime);
    
    _audioPlayer->getAudioQueue()->synchronize(currentRTPTime, currentTime, nextRTPTime);
    
}

void RTPReceiver::_processTimingResponsePacket(RTPPacket* packet) {
    
    assert(packet != NULL);
    
    double currentTime = CACurrentMediaTime();
    double referenceTime = ntpTimeToDouble(*((NTPTime*)&packet->packet_data[4]));
    double receivedTime = ntpTimeToDouble(*((NTPTime*)&packet->packet_data[12]));
    double sendTime = ntpTimeToDouble(*((NTPTime*)&packet->packet_data[20]));

    /*
    log(LOG_INFO, "current time: %1.6f", currentTime);
    log(LOG_INFO, "reference time: %1.6f", referenceTime);
    log(LOG_INFO, "received time: %1.6f", receivedTime);
    log(LOG_INFO, "send time: %1.6f", sendTime);
    */
     
    double delay = ((currentTime - referenceTime) - (sendTime - receivedTime)) / 2;
    double clientTime = receivedTime + (sendTime - receivedTime) + delay;
        
    log(LOG_INFO, "Client time is %1.6f (peer delay: %1.6f)", clientTime, delay);
    
    _audioPlayer->setClientTime(clientTime);
    
    _timeResponseCount++;
    
    if (_timeResponseCount < 3) // On connection start we send three packets in row
        this->_sendTimingRequest();
    else
        if (_timeResponseCount == 3)
            _timerCond.signal();
    
}

void RTPReceiver::_sendPacket(const char* buffer, size_t len, SocketEndPoint* remoteEndPoint, Socket* sock) {
    
    assert(buffer != NULL && len > 0 && remoteEndPoint != NULL && sock != NULL);
    
    sock->SendTo(remoteEndPoint, buffer, len);
        
}

void RTPReceiver::_sendTimingRequest() {
    
    RTPTimingPacket pckt;
    
    memset(&pckt, 0, RTPTimingPacketSize);
    
    pckt.a = 0x80;
    pckt.b = RTP_TIMING_REQUEST | ~RTP_PAYLOAD_TYPE;
    
    pckt.seq_num = 0x0700;
    
    double send_time = CACurrentMediaTime();
    pckt.send_time = doubleToNtpTime(send_time);
    
    _sendPacket((char*)&pckt, RTPTimingPacketSize, _remoteTimingEndPoint, _timingSock);
    
    log(LOG_INFO, "Timing synchronization request sent (@ %1.6f)", send_time);
    
}

void RTPReceiver::_sendResendRequest(unsigned short seqNum, unsigned short count) {
    
    assert(count > 0);
    
    RTPResendPacket pckt;
    
    memset(&pckt, 0, sizeof(RTPResendPacket));
    
    pckt.a = 0x80;
    pckt.b = RTP_RANGE_RESEND_REQUEST | ~RTP_PAYLOAD_TYPE;
    pckt.seq_num = pckt.count = htons(count);
    pckt.missed_seq = htons(seqNum);
    
    _sendPacket((char*)&pckt, RTPResendPacketSize, _remoteControlEndPoint, _controlSock);
    
    log(LOG_INFO, "Requested packet resend (seq: %d / count %d)", seqNum, count);
    
}

void* RTPReceiver::_synchronizationLoopKickStarter(void* t) {
    
    assert(t != NULL);
    
    pthread_setname_np("Synchronization Loop");
    ((RTPReceiver*)t)->_synchronizationLoop();
    pthread_exit(0);
    
}

void RTPReceiver::_synchronizationLoop() {

    _timerMutex.lock();

    while (_timerRunning) {
        
        _sendTimingRequest();
        
        _syncCond.timedWait(&_timerMutex, 2000);
        
    }
    
    _timerMutex.unlock();
    
}

void RTPReceiver::_startSynchronizationLoop() {
    
    _timerMutex.lock();
    
    if (!_timerRunning) {
        
        _timerRunning = true;
        pthread_create(&_timerThread, NULL, _synchronizationLoopKickStarter, this);
        
        log(LOG_INFO, "Synchronization loop started");
        
    }
    
    _timerMutex.unlock();
    
}

void RTPReceiver::_stopSynchronizationLoop() {
    
    _timerMutex.lock();
    
    if (_timerRunning) {
        _timerRunning = false;
        _syncCond.signal();
        log(LOG_INFO, "Synchronization loop stopped");
    }
    
    _timerMutex.unlock();
    
    pthread_join(_timerThread, NULL);

}

void RTPReceiver::_processAudioPacket(RTPPacket* packet) {
    
    assert(packet != NULL);
    
    uint32_t rtpTime = ntohl(*(uint32_t*)&packet->packet_data[0]);    
    uint16_t cSeq = (int)packet->seq_num;
    
    unsigned char* packetAudioData = &packet->packet_data[8];
    uint32_t len = packet->packet_data_length - 8;
    size_t encryptedLen = len & ~0xf;
    
    if (_streamIsEncrypted) {
        
        unsigned char iv[16];
        unsigned char out[encryptedLen];
        
        memcpy(iv, _aesIv, 16);
        
        AES_cbc_encrypt(packetAudioData, out, encryptedLen, &_aes, iv, AES_DECRYPT);
        memcpy((char*)packetAudioData, out, encryptedLen);
        
    }
    
    int missingCount = _audioPlayer->addAudio(packetAudioData, len, cSeq, rtpTime);
    if (missingCount > 0)
        _sendResendRequest(cSeq - missingCount, missingCount);
    
    int missingSeq;
    while ((missingCount = _audioPlayer->getAudioQueue()->getNextMissingWindow(&missingSeq)) > 0)
        _sendResendRequest(cSeq, missingCount);
    
}

void* RTPReceiver::_streamLoopKickStarter(void* t) {
    
    assert(t != NULL);
    
    _streamLoopKickStarterHelper* helper = (_streamLoopKickStarterHelper*)t;
    pthread_setname_np(helper->name);
    helper->t->_streamLoop(helper->sock);
    free(helper);
    pthread_exit(0);
    
}

void RTPReceiver::_streamLoop(Socket* sock) {
    
    assert(sock != NULL);
    
    log(LOG_INFO, "Stream loop started");
    
    for (;;) {
        
        unsigned char buffer[16384];
        long read = sock->Receive(buffer, 16384);
        
        if (read > 0 && sock->GetRemoteEndPoint()->compareWithAddress(_connection->getRemoteEndPoint())) {
            
            RTPPacket packet = readHeader(buffer, read);
            
            if (packet.payload_type == RTP_AUDIO_RESEND_DATA)
                packet = readHeader(&buffer[4], read - 4);
            
            switch (packet.payload_type) {
               case RTP_TIMING_RESPONSE:
                    _processTimingResponsePacket(&packet);
                    break;
                case RTP_SYNC:
                    _processSyncPacket(&packet);
                    break;
                case RTP_AUDIO_DATA:
                    _processAudioPacket(&packet);
                    break;
                default:
                    log(LOG_ERROR, "Received unknown packet");
                    break;
            }
            
        } else
            break;
        
    }
    
    sock->Close();
    
}

Socket* RTPReceiver::_setupSocket(unsigned short* port) {
    
    SafeDR(port, 0);
    
    Socket* ret = new Socket(true);
    
    if (ret >= 0) {
        
        unsigned short p;
        for (p = 6000 ; p < 6100 ; p++) {
            SocketEndPoint* ep = _connection->getLocalEndPoint()->copy(p);
            if (ret->Bind(ep)) {
                delete ep;
                SafeDR(port, p);
                log(LOG_INFO, "Setup socket on port %u", p);
                return ret;
            } else
                delete ep;
        }
        
        log(LOG_ERROR, "Unable to bind socket.");
        
    } else
        log(LOG_ERROR, "Unable to create socket.");
    
    delete ret;
    
    return NULL;
        
}

unsigned short RTPReceiver::setup(unsigned short* timingPort, unsigned short* controlPort) {
    
    assert(timingPort != NULL && controlPort != NULL && *timingPort != 0 && *controlPort != 0);
    
    unsigned short ret = 0;
    log(LOG_INFO, "Setting up server socket");
    if ((_serverSock = _setupSocket(&ret)) != NULL) {
        
        if (controlPort != NULL && controlPort > 0) {
            _remoteControlEndPoint = _connection->getRemoteEndPoint()->copy(*controlPort);
            log(LOG_INFO, "Setting up control socket");
            _controlSock = _setupSocket(controlPort);
        }
        
        if (timingPort != NULL && timingPort > 0) {
            _remoteTimingEndPoint = _connection->getRemoteEndPoint()->copy(*timingPort);
            log(LOG_INFO, "Setting up timing socket");
            _timingSock = _setupSocket(timingPort);
        }
        
        return ret;
        
    }
    
    return 0;
    
}

pthread_t RTPReceiver::_start(Socket* socket, const char* name) {
    
    assert(socket != NULL);
    
    _streamLoopKickStarterHelper* h = (_streamLoopKickStarterHelper*)malloc(sizeof(_streamLoopKickStarterHelper));
    h->t = this;
    h->sock = socket;
    strcpy(h->name, name);
    
    pthread_t ret;
    pthread_create(&ret, NULL, _streamLoopKickStarter, h);
    
    return ret;
    
}

void RTPReceiver::start(unsigned short startSeq) {
    
    __globalMutex.lock();
    __globalReceiver = this;
    __globalMutex.unlock();
    
    _audioPlayer->start();
    
    log(LOG_INFO, "Started RTP receiver with initial sequence number: %d", startSeq);

    if (_serverSock != NULL) {
        
        _serverSockThread = _start(_serverSock, "Stream Socket");
        if (_timingSock != NULL)
            _timingSockThread = _start(_timingSock, "Timing Socket");
        if (_controlSock != NULL)
            _controlSockThread = _start(_controlSock, "Control Socket");
        
        _sendTimingRequest();
        
        log(LOG_INFO, "Waiting for synchronization");
        
        _timerMutex.lock();
        if (_timerCond.timedWait(&_timerMutex, 4000))
            log(LOG_INFO, "Synchronization incomplete");
        else
            log(LOG_INFO, "Synchronization complete");
        _timerMutex.unlock();
        
        
    } else
        log(LOG_ERROR, "RTP receiver was not setup before start");
    
}

void RTPReceiver::stop() {

    _stopSynchronizationLoop();
    
    if (_serverSock != NULL) {
        _serverSock->Close();
        pthread_join(_serverSockThread, NULL);
        delete _serverSock;
    }
    if (_timingSock != NULL) {
        _timingSock->Close();
        pthread_join(_timingSockThread, NULL);
        delete _timingSock;
    }
    if (_controlSock != NULL) {
        _controlSock->Close();
        pthread_join(_controlSockThread, NULL);
        delete _controlSock;
    }
    
    delete _remoteTimingEndPoint;
    delete _remoteControlEndPoint;
    
    _serverSock = _timingSock = _controlSock = NULL;
    _serverSockThread = _timingSockThread = _controlSockThread = NULL;
    _remoteTimingEndPoint = _remoteControlEndPoint = NULL;
    
    __globalMutex.lock();
    __globalReceiver = NULL;
    __globalMutex.unlock();
    
}

void RTPReceiver::_notificationReceived(Notification* notification) {
    
    if (*notification == AudioQueue::flushNotificationName)
        _stopSynchronizationLoop();
    else if (*notification == AudioQueue::syncNotificationName)
        _startSynchronizationLoop();
    
}
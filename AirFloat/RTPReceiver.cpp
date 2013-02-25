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

extern "C" {
#include "log.h"
#include "mutex.h"
}

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

static mutex_p __globalMutex = NULL;
static RTPReceiver* __globalReceiver = NULL;

typedef struct {
    
    uint32_t integer;
    uint32_t fraction;
    
} NTPTime;

typedef struct {
    
    RTPReceiver* t;
    socket_p sock;
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
    
    mutex_destroy(_timerMutex);
    condition_destroy(_timerCond);
    condition_destroy(_syncCond);
    thread_destroy(_timerThread);
    
    notification_center_remove_observer(NULL, this);
    
}

void RTPReceiver::_setup(RAOPConnection* connection, int* fmtp, int fmtpLen) {
    
    assert(connection != NULL && fmtp != NULL);

    _serverSock = _timingSock = _controlSock = 0;
    _remoteTimingEndPoint = _remoteControlEndPoint = NULL;
    
    for (int i = 0 ; i < 4 ; i++)
        _session[i] = (char)rand();
    
    _connection = connection;
    
    _streamIsEncrypted = false;
    
    _timeResponseCount = 0;
    
    _audioPlayer = new AudioPlayer(fmtp, fmtpLen);
    
    _timerRunning = false;
    
    _emulatedSeqNo = 0;
    
    _timerMutex = mutex_create();
    _timerCond = condition_create();
    _syncCond = condition_create();
    
    if (__globalMutex == NULL)
        __globalMutex = mutex_create();
    
    notification_center_add_observer(RTPReceiver::_notificationReceived, this, AudioQueue::flushNotificationName, NULL);
    notification_center_add_observer(RTPReceiver::_notificationReceived, this, AudioQueue::syncNotificationName, NULL);
    
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
    
    mutex_lock(__globalMutex);
    bool ret = !(__globalReceiver != NULL && __globalReceiver->started());
    mutex_unlock(__globalMutex);
    
    return ret;
    
}

RTPReceiver* RTPReceiver::getStreamingReceiver() {
    
    if (__globalMutex == NULL)
        __globalMutex = mutex_create();
    
    mutex_lock(__globalMutex);
    RTPReceiver* ret = __globalReceiver;
    mutex_unlock(__globalMutex);
    
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
    
    log_message(LOG_INFO, "Sync packet (Playhead frame: %u - current time: %1.6f - next frame: %u)", currentRTPTime, currentTime, nextRTPTime);
    
    _audioPlayer->getAudioQueue()->synchronize(currentRTPTime, currentTime, nextRTPTime);
    
}

void RTPReceiver::_processTimingResponsePacket(RTPPacket* packet) {
    
    assert(packet != NULL);
    
    double currentTime = CACurrentMediaTime();
    double referenceTime = ntpTimeToDouble(*((NTPTime*)&packet->packet_data[4]));
    double receivedTime = ntpTimeToDouble(*((NTPTime*)&packet->packet_data[12]));
    double sendTime = ntpTimeToDouble(*((NTPTime*)&packet->packet_data[20]));

    double delay = ((currentTime - referenceTime) - (sendTime - receivedTime)) / 2;
    double clientTime = receivedTime + (sendTime - receivedTime) + delay;
        
    log_message(LOG_INFO, "Client time is %1.6f (peer delay: %1.6f)", clientTime, delay);
    
    _audioPlayer->setClientTime(clientTime);
    
    _timeResponseCount++;
    
    if (_timeResponseCount < 3) // On connection start we send three packets in row
        this->_sendTimingRequest();
    else
        if (_timeResponseCount == 3)
            condition_signal(_timerCond);
    
}

void RTPReceiver::_sendPacket(const char* buffer, size_t len, struct sockaddr* remoteEndPoint, RTPSocket* sock) {
    
    assert(buffer != NULL && len > 0 && remoteEndPoint != NULL && sock != NULL);
    
    sock->sendTo(remoteEndPoint, buffer, len);
        
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
    
    log_message(LOG_INFO, "Timing synchronization request sent (@ %1.6f)", send_time);
    
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
    
    log_message(LOG_INFO, "Requested packet resend (seq: %d / count %d)", seqNum, count);
    
}

void RTPReceiver::_synchronizationLoopKickStarter(void* t) {
    
    assert(t != NULL);
    
    pthread_setname_np("Synchronization Loop");
    ((RTPReceiver*)t)->_synchronizationLoop();
    pthread_exit(0);
    
}

void RTPReceiver::_synchronizationLoop() {
    
    mutex_lock(_timerMutex);

    while (_timerRunning) {
        
        _sendTimingRequest();
        
        condition_times_wait(_timerCond, _timerMutex, 2000);
        
    }
    
    mutex_unlock(_timerMutex);
    
}

void RTPReceiver::_startSynchronizationLoop() {
    
    mutex_lock(_timerMutex);
    
    if (!_timerRunning) {
        
        _timerRunning = true;
        _timerThread = thread_create(_synchronizationLoopKickStarter, this);
        
        log_message(LOG_INFO, "Synchronization loop started");
        
    }
    
    mutex_unlock(_timerMutex);
    
}

void RTPReceiver::_stopSynchronizationLoop() {
    
    mutex_lock(_timerMutex);
    
    if (_timerRunning) {
        _timerRunning = false;
        condition_signal(_syncCond);
        log_message(LOG_INFO, "Synchronization loop stopped");
    }
    
    mutex_unlock(_timerMutex);
    
    thread_join(_timerThread);
    
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

uint32_t RTPReceiver::_dataReceivedHelper(RTPSocket* rtpSocket, socket_p socket, const char* buffer, uint32_t size, void* ctx) {
    
    /* AirTunes v2 runs UDP. */
    if (socket_is_udp(socket))
        return ((RTPReceiver*)ctx)->_dataReceivedUDP(rtpSocket, socket, buffer, size);
    
    return ((RTPReceiver*)ctx)->_dataReceivedTCP(rtpSocket, socket, buffer, size);
    
}

uint32_t RTPReceiver::_dataReceivedTCP(RTPSocket* rtpSocket, socket_p socket, const char* buffer, uint32_t size) {
    
    uint32_t read = 0;
    
    while (read < size) {
        
        if (size < 4)
            break;
        
        uint32_t packetSize = ntohs(*((uint32_t*)&buffer[read + 2]));
        if (size - read < packetSize + 4)
            break;
        
        read += 4;
        
        if (buffer[read] == '\xf0' && buffer[read + 1] == '\xff') {
            RTPPacket packet = readHeader((const unsigned char*)&buffer[read], packetSize);
            
            packet.seq_num = _emulatedSeqNo++;
            
            _processAudioPacket(&packet);
        }
        
        read += packetSize;
        
        assert(read <= size);
        
    }
    
    return read;
    
}

uint32_t RTPReceiver::_dataReceivedUDP(RTPSocket* rtpSocket, socket_p socket, const char* buffer, uint32_t size) {
    
    RTPPacket packet = readHeader((const unsigned char*)buffer, size);
    
    if (packet.payload_type == RTP_AUDIO_RESEND_DATA)
        packet = readHeader((const unsigned char*) &buffer[4], size - 4);
    
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
            log_message(LOG_ERROR, "Received unknown packet");
            break;
    }
    
    return size;
    
}

RTPSocket* RTPReceiver::_setupSocket(char* name, unsigned short* port) {
    
    SafeDR(port, 0);
    
    RTPSocket* ret = new RTPSocket(name, _connection->getRemoteEndPoint());
    
    unsigned short p;
    for (p = 6000 ; p < 6100 ; p++) {
        struct sockaddr* ep = sockaddr_copy(_connection->getLocalEndPoint());
        sockaddr_set_port(ep, p);
        if (ret->setup(ep)) {
            ret->setDataReceivedCallback(_dataReceivedHelper, this);
            SafeDR(port, p);
            log_message(LOG_INFO, "Setup socket on port %u", p);
            sockaddr_destroy(ep);
            return ret;
        }
        sockaddr_destroy(ep);
    }
    
    log_message(LOG_ERROR, "Unable to bind socket.");
    
    delete ret;
    
    return NULL;
        
}

unsigned short RTPReceiver::setup(unsigned short* timingPort, unsigned short* controlPort) {
    
    unsigned short ret = 0;
    log_message(LOG_INFO, "Setting up server socket");
    if ((_serverSock = _setupSocket((char*)"Stream socket", &ret)) != NULL) {
        
        if (controlPort != NULL && *controlPort > 0) {
            
            _remoteControlEndPoint = sockaddr_copy(_connection->getRemoteEndPoint());
            sockaddr_set_port(_remoteControlEndPoint, *controlPort);
            log_message(LOG_INFO, "Setting up control socket");
            _controlSock = _setupSocket((char*)"Control socket", controlPort);
            
        }
        
        if (timingPort != NULL && *timingPort > 0) {
            
            _remoteTimingEndPoint = sockaddr_copy(_connection->getRemoteEndPoint());
            sockaddr_set_port(_remoteTimingEndPoint, *timingPort);
            log_message(LOG_INFO, "Setting up timing socket");
            _timingSock = _setupSocket((char*)"Timing socket", timingPort);
            
        } else
            _audioPlayer->disableSynchronization();
        
    }
    
    return ret;
    
}

bool RTPReceiver::start(unsigned short startSeq) {
    
    mutex_lock(__globalMutex);
    __globalReceiver = this;
    mutex_unlock(__globalMutex);
    
    _audioPlayer->start();
        
    log_message(LOG_INFO, "Started RTP receiver with initial sequence number: %d", startSeq);

    if (_serverSock != NULL) {
        
        if (_timingSock != NULL) {
        
            _sendTimingRequest();
            
            log_message(LOG_INFO, "Waiting for synchronization");
            
            mutex_lock(_timerMutex);
            bool ret = !condition_times_wait(_timerCond, _timerMutex, 5000);
            if (!ret)
                log_message(LOG_INFO, "Synchronization incomplete");
            else
                log_message(LOG_INFO, "Synchronization complete");
            mutex_unlock(_timerMutex);
            
            return ret;
            
        }
        
        return true;
        
    } else
        log_message(LOG_ERROR, "RTP receiver was not setup before start");
    
    return true;
    
}

void RTPReceiver::stop() {

    _stopSynchronizationLoop();
    
    if (_serverSock != NULL)
        delete _serverSock;
    
    if (_timingSock != NULL)
        delete _timingSock;
    
    if (_controlSock != NULL)
        delete _controlSock;
    
    if (_remoteTimingEndPoint)
        sockaddr_destroy(_remoteTimingEndPoint);
    if (_remoteControlEndPoint)
        sockaddr_destroy(_remoteControlEndPoint);
    
    _serverSock = _timingSock = _controlSock = NULL;
    _remoteTimingEndPoint = _remoteControlEndPoint = NULL;
    
    mutex_lock(__globalMutex);
    __globalReceiver = NULL;
    mutex_unlock(__globalMutex);
    
}

void RTPReceiver::_notificationReceived(notification_p notification, void* ctx) {
    
    RTPReceiver* i = (RTPReceiver*)ctx;
    
    if (i->_timingSock) {
        
        if (notification_get_name(notification) == AudioQueue::flushNotificationName)
            i->_stopSynchronizationLoop();
        else if (notification_get_name(notification) == AudioQueue::syncNotificationName)
            i->_startSynchronizationLoop();
        
    }
    
}

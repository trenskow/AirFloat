//
//  AudioQueue.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AudioQueue_h
#define AirFloat_AudioQueue_h

#include <stdint.h>

#include "Mutex.h"
#include "Condition.h"
#include "AudioPacket.h"

class AudioPlayer;

class AudioQueue {
    
    friend class AudioPlayer;
    
public:
    AudioQueue(double packetSize, double frameSize, double sampleRate);
    ~AudioQueue();
    
    bool hasAvailablePacket();
    double getPacketTime();
    void getAudio(void* buffer, uint32_t* size, double* time, uint32_t* sampleTime);
    void discardPacket();
    void synchronize(uint32_t currentSampleTime, double currentTime, uint32_t nextSampleTime);
    int getNextMissingWindow(int* seqNo);
    int getQueuePacketCount();
    bool awaitAvailabilty();
    
    static const char* flushNotificationName;
    static const char* syncNotificationName;

private:
    
    int _addAudioPacket(void* buffer, uint32_t size, int seqNo, uint32_t sampleTime);
    void _checkQueueConsistency();
    AudioPacket* _createPacket(AudioPacketState state);
    void _disposePacket(AudioPacket* packet);
    void _addPacketToQueueTail(AudioPacket* packet);
    AudioPacket* _addEmptyPacket();
    AudioPacket* _popQueueFromHead(bool keepQueueFilled = true);
    void _flush();
    int _handleSequenceOverflow(int seqNo);
    void _disableSynchronization();
    
    double _convertTime(uint32_t fromSampleTime, double fromTime, uint32_t toSampleTime);
    
    AudioPacket* _queueHead;
    AudioPacket* _queueTail;
    
    double _packetSize; // Frames in packet
    double _frameSize;  // Size of frame
    double _sampleRate;
    
    bool _synchronizationEnabled;
    bool _synchronizationNotificationSent;
    
    int _foldCount;
    
    int _queueCount;
    int _missingCount;
    int _queueFrameCount;
    
    double _lastKnowSampleTime;
    double _lastKnowSampleTimesTime;
    
    Mutex _mutex;
    Condition _condition;
    
    bool _disposed;
    
};

#endif

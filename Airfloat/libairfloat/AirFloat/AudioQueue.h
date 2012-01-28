//
//  AudioQueue.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AudioQueue_h
#define AirFloat_AudioQueue_h

#include <pthread.h>
#include <stdint.h>

class  AudioPlayer;

typedef enum {
    
    kAudioPacketStateComplete = 0,
    kAudioPacketStateWaiting,
    kAudioPacketStateMissing,
    kAudioPacketStateRequested
    
} AudioPacketState;

typedef struct AudioPacket {
    
    void* buffer;
    size_t bufferSize;
    int seqNo;
    uint32_t sampleTime;
    double time;
    AudioPacketState state;
    
    AudioPacket* next;
    AudioPacket* prev;
    
} AudioPacket;

class AudioQueue;

typedef int(*audioQueueAddPacketClbk)(AudioQueue* queue, void* buffer, int size, void* outBuffer, int outBufferSize, void* ctx);
typedef void(*audioQueueSimpleClbk)(AudioQueue* queue, void* ctx);

class AudioQueue {
    
    friend class AudioPlayer;
    
public:
    AudioQueue(double packetSize, double frameSize, double sampleRate);
    ~AudioQueue();
    
    bool hasAvailablePacket();
    double getPacketTime();
    void getPacket(void* buffer, int* size, double* time, uint32_t* sampleTime);
    void synchronize(uint32_t currentSampleTime, double currentTime, uint32_t nextSampleTime);
    int getNextMissingWindow(int* seqNo);
    
    void setAddPacketCallback(audioQueueAddPacketClbk clbk, void* ctx);
    void setFlushCallback(audioQueueSimpleClbk clbk, void* ctx);
    void setSyncCallback(audioQueueSimpleClbk clbk, void* ctx);
    
private:
    
    int _addAudioPacket(void* buffer, int size, int seqNo, uint32_t sampleTime);
    void _flush();
    void _checkQueueConsistency();
    AudioPacket* _createPacket(AudioPacketState state = kAudioPacketStateWaiting);
    void _disposePacket(AudioPacket* packet);
    void _addPacketToQueueTail(AudioPacket* packet);
    AudioPacket* _addEmptyPacket(AudioPacketState state = kAudioPacketStateWaiting);
    AudioPacket* _replacePacket(AudioPacket* oldPacket, AudioPacket* newPacket);
    AudioPacket* _popQueueFromHead();
    
    double _convertTime(uint32_t fromSampleTime, double fromTime, uint32_t toSampleTime);

    AudioPacket* _queueHead;
    AudioPacket* _queueTail;
    
    double _packetSize; // Frames in packet
    double _frameSize;  // Size of frame
    double _sampleRate;
    
    int _foldCount;
    
    int _queueCount;
    int _missingCount;
    
    bool _isSynced;
    
    audioQueueAddPacketClbk _addPacketClbk;
    void* _addPacketClbkCtx;
    audioQueueSimpleClbk _flushClbk;
    void* _flushClbkCtx;
    audioQueueSimpleClbk _syncClbk;
    void* _syncClbkCtx;
    
    pthread_mutex_t _mutex;
    
};

#endif

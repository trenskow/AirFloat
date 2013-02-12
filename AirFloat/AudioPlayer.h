//
//  AudioPlayer.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AudioPlayer_h
#define AirFloat_AudioPlayer_h

#define BUFFER_COUNT 10

#include "Mutex.h"
#include "AudioConverter.h"
#include "AudioQueue.h"
#include <AudioToolbox/AudioToolbox.h>

#define CLIENT_SERVER_DIFFERENCE_BACKLOG 10

class AudioPlayer {
        
public:
    AudioPlayer(int* fmts, int fmtsSize);
    ~AudioPlayer();
    
    void start();
    void flush(int32_t lastSeq = -1);
    void setClientTime(double time);
    
    void setVolume(double volume);
    
    int addAudio(void* buffer, uint32_t size, int seqNo, uint32_t sampleTime);
    
    double getSampleRate();
    
    AudioQueue* getAudioQueue();
    
private:
    
    void _audioOutputCallback(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
    static void _audioOutputCallbackHelper(void* aqData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
    
    int _srcFramesPerPacket;
    struct {
        AudioStreamBasicDescription outDesc;
        AudioQueueTimelineRef timeline;
        AudioConverter* losslessConverter;
        AudioConverter* speedUpConverter;
        AudioConverter* slowDownConverter;
        AudioQueueRef queue;
        AudioQueueBufferRef buffers[BUFFER_COUNT];
        AudioStreamPacketDescription* pckDesc;
    } _audio;
    
    double _clientServerDifferenceHistory[10];
    int _clientServerDifferenceHistoryCount;
    
    double _clientServerDifference;
    double _nextPacketTime;
    bool _outputIsHomed;
    
    Mutex _timeMutex;

    AudioQueueParameterValue _preFlushVolume;
    
    AudioQueue* _audioQueue;
    
    int32_t _flushedSeq;
    
    bool _skipPacket;
    
    Mutex _packetMutex;
            
};

#endif

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

#include <pthread.h>
#include "AppleLosslessAudioConverter.h"
#include "AudioQueue.h"
#include <AudioToolbox/AudioToolbox.h>

#define CLIENT_SERVER_DIFFERENCE_BACKLOG 10

typedef enum {
    
    kAudioPlayerSpeedSlowDown = -1,
    kAudioPlayerSpeedNormal = 0,
    kAudioPlayerSpeedSpeedUp = 1
    
} AudioPlayerSpeed;

class AudioPlayer {
    
public:
    AudioPlayer(int* fmts, int fmtsSize);
    ~AudioPlayer();
    
    void start();
    void flush();
    void setClientTime(double time);
    
    void setVolume(double volume);
    
    int addAudio(void* buffer, int size, int seqNo, uint32_t sampleTime);
    
    AudioQueue* getAudioQueue();
    
private:
    
    void _audioQueueCallback(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
    static void _audioQueueCallbackHelper(void* aqData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
    
    int _srcFramesPerPacket;
    struct {
        AudioStreamBasicDescription outDesc;
        AudioQueueTimelineRef timeline;
        AppleLosslessAudioConverter* losslessConverter;
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
    
    pthread_mutex_t _timeMutex;

    AudioQueue* _audioQueue;
    AudioPlayerSpeed _speed;
            
};

#endif

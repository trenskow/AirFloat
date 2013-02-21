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
#include "AudioGraph.h"
#include "AudioUnitMixer.h"
#include "AudioUnitVariSpeed.h"
#include "AudioUnitOutput.h"
#include <AudioToolbox/AudioToolbox.h>

#define CLIENT_SERVER_DIFFERENCE_BACKLOG 10

using namespace Audio;

class AudioPlayer {
    
public:
    AudioPlayer(int* fmts, int fmtsSize);
    ~AudioPlayer();
    
    void start();
    void flush(int32_t lastSeq = -1);
    void setClientTime(double time);
    
    void setVolume(double volume);
    void disableSynchronization();
    
    int addAudio(void* buffer, uint32_t size, int seqNo, uint32_t sampleTime);
    
    double getSampleRate();
    
    AudioQueue* getAudioQueue();
    
private:
    
    OSStatus _renderCallback (AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList  *ioData);
    static OSStatus _renderCallbackHelper (void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList  *ioData);

    int _srcFramesPerPacket;
    struct {
        AudioConverter* losslessConverter;
        AudioStreamBasicDescription outDesc;
        struct {
            Graph *graph;
            UnitMixer *mixerUnit;
            VarispeedUnit *speedUnit;
            UnitOutput *outputUnit;
        } graph;
    } _audio;
    
    double _clientServerDifferenceHistory[10];
    int _clientServerDifferenceHistoryCount;
    
    double _clientServerDifference;
    bool _outputIsHomed;
    
    Mutex _timeMutex;
    
    AudioQueueParameterValue _preFlushVolume;
    
    bool _synchronizationEnabled;
    AudioQueue* _audioQueue;
    
    int32_t _flushedSeq;
    
    bool _skipPacket;
    
    Mutex _packetMutex;
            
};

#endif

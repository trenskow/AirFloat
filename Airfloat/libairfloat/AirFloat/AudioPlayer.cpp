//
//  AudioPlayer.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <sys/utsname.h>
#include <QuartzCore/QuartzCore.h>

extern "C" {
#include "log.h"
}

#include "CAHostTimeBase.h"

#include "AppleLosslessSoftwareAudioConverter.h"
#include "AppleLosslessAudioConverter.h"

#include "Settings.h"

#include "AudioPlayer.h"

#ifndef MIN
#define MIN(x,y) (x < y ? x : y)
#endif
#define FROM_CLIENT_TIME(x) (x + _clientServerDifference)

using namespace Audio;

AudioPlayer::AudioPlayer(int* fmts, int fmtsSize) {
    
    assert(fmts != NULL && fmtsSize > 0);
    
    uint32_t maximumFramesPerSlice = 4096;
    
#if TARGET_OS_IPHONE
    UInt32 category = kAudioSessionCategory_MediaPlayback;
    AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
    float bufferLength = maximumFramesPerSlice / 44100.0f;
    AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof(bufferLength), &bufferLength);
    AudioSessionSetActive(true);
#endif
        
    bzero(&_audio, sizeof(_audio));
    
    _srcFramesPerPacket = fmts[0];
    
    _clientServerDifference = 0;
    _outputIsHomed = false;
    _synchronizationEnabled = true;
    
    bzero(_clientServerDifferenceHistory, sizeof(double) * CLIENT_SERVER_DIFFERENCE_BACKLOG);
    _clientServerDifferenceHistoryCount = 0;
        
    _audio.losslessConverter = new AppleLosslessAudioConverter(fmts, fmtsSize);
    _audio.outDesc = _audio.losslessConverter->getDestDescription();
    
    _preFlushVolume = 1.0;
    _flushedSeq = -1;
    
    /* Find out if Varispeed is supported */
    struct utsname name;
    uname(&name);
    
    double version = strtod(name.release, NULL);
    
    Graph *graph = _audio.graph.graph = new Graph(maximumFramesPerSlice);
    
    _audio.graph.mixerUnit = new UnitMixer(1, graph);
    _audio.graph.outputUnit = new UnitOutput(graph);
    
    if (version >= 11) { /* Darwin 11 is iOS 5. Varispeed is only available in iOS 5+. */
        _audio.graph.speedUnit = new VarispeedUnit(graph);
        _audio.graph.outputUnit->connect(_audio.graph.speedUnit, 0, 0);
        _audio.graph.speedUnit->connect(_audio.graph.mixerUnit, 0, 0);
    } else {
        _audio.graph.speedUnit = NULL;
        _audio.graph.outputUnit->connect(_audio.graph.mixerUnit, 0, 0);
    }
        
    _audio.graph.mixerUnit->setAudioDescriptionForInputBus(_audio.outDesc, 0);
    
    _audio.graph.mixerUnit->setInputCallback(0, _renderCallbackHelper, this);
    
    _audio.graph.mixerUnit->setVolume(1.0f, 0);
    _audio.graph.mixerUnit->setEnabled(TRUE, 0);
    if (_audio.graph.speedUnit != NULL)
        _audio.graph.speedUnit->setPlaybackRate(1.0);
    
    _audio.graph.graph->inititializeAndUpdate();
    
    _audioQueue = new AudioQueue(fmts[0], _audio.outDesc.mBytesPerFrame, _audio.outDesc.mSampleRate);
    
    _timeMutex = mutex_create();
    _packetMutex = mutex_create();
    
}

AudioPlayer::~AudioPlayer() {
    
    _audio.graph.graph->setRunning(false);
    
    delete _audio.graph.mixerUnit;
    delete _audio.graph.speedUnit;
    delete _audio.graph.outputUnit;
    delete _audio.graph.graph;
    
    delete _audioQueue;
    delete _audio.losslessConverter;
    
    mutex_destroy(_timeMutex);
    mutex_destroy(_packetMutex);
    
}

void AudioPlayer::start() {
    
    _audio.graph.graph->setRunning(true);
    
}

void AudioPlayer::flush(int lastSeq) {
    
    mutex_lock(_timeMutex);
    
    mutex_lock(_packetMutex);
    
    _audioQueue->_flush();
    _flushedSeq = lastSeq;
    
    mutex_unlock(_packetMutex);
    
    _audio.graph.mixerUnit->setVolume(.0f, 0);
    
    _outputIsHomed = false;
    
    mutex_unlock(_timeMutex);
        
}

void AudioPlayer::setVolume(double volume) {
    
    _preFlushVolume = volume;
    _audio.graph.mixerUnit->setVolume(volume, 0);
    
}

void AudioPlayer::disableSynchronization() {
    
    _synchronizationEnabled = false;
    _audioQueue->_disableSynchronization();
    
}

void AudioPlayer::setClientTime(double time) {
    
    mutex_lock(_timeMutex);
    
    UInt64 currentTime = CAHostTimeBase::GetCurrentTimeInNanos();
    
    _clientServerDifference = 0;
    
    _clientServerDifferenceHistoryCount = MIN(_clientServerDifferenceHistoryCount + 1, CLIENT_SERVER_DIFFERENCE_BACKLOG);
    for (int i = _clientServerDifferenceHistoryCount - 1 ; i > 0 ; i--)
        _clientServerDifference += (_clientServerDifferenceHistory[i] = _clientServerDifferenceHistory[i-1]);
    
    _clientServerDifference += (_clientServerDifferenceHistory[0] = currentTime / 1000000000.0 - time);
    
    _clientServerDifference /= (double)_clientServerDifferenceHistoryCount;
    
    log_message(LOG_INFO, "Time difference: %1.5f", _clientServerDifference);
    
    mutex_unlock(_timeMutex);
    
}

AudioQueue* AudioPlayer::getAudioQueue() {
    
    return _audioQueue;
    
}

OSStatus AudioPlayer::_renderCallback (AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList  *ioData) {
    
    static int run = 0;
    
    mutex_lock(_timeMutex);
    
    bzero(ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize);
    
    if (_audioQueue->hasAvailablePacket()) {
        
        double packetDuration = inNumberFrames / _audio.outDesc.mSampleRate;
        double packetStartTime = CAHostTimeBase::ConvertToNanos(inTimeStamp->mHostTime) / 1000000000.0;
        double queueTime = FROM_CLIENT_TIME(_audioQueue->getPacketTime());
        
        uint32_t dataOffset = 0;
        
        if (!_outputIsHomed) {
            
            if (_synchronizationEnabled) {
                                
                /* We calculate for next frame */
                double packetEndTime = packetStartTime + packetDuration;
                
                if (queueTime < packetEndTime) {
                    
                    double packetPos = packetEndTime - queueTime;
                    dataOffset = floor(inNumberFrames * packetPos) * _audio.outDesc.mBytesPerFrame;
                    _outputIsHomed = true;
                    log_message(LOG_INFO, "Output is homed (%d - %d/%d)", run, dataOffset, ioData->mBuffers[0].mDataByteSize);
                    
                } else if (queueTime < packetStartTime) {
                    
                    _audioQueue->getAudio(ioData->mBuffers[0].mData, (uint32_t *)&ioData->mBuffers[0].mDataByteSize, NULL, NULL);
                    _outputIsHomed = true;
                    log_message(LOG_INFO, "Output is homed - without catching the exact moment.");
                    
                }
                
            } else
                _outputIsHomed = true;
            
        }
        
        if (_outputIsHomed) {
            
            char* buf = &((char*)ioData->mBuffers[0].mData)[dataOffset];
            uint32_t size = ioData->mBuffers[0].mDataByteSize - dataOffset;
            _audioQueue->getAudio(buf, &size, NULL, NULL);
            
            if (_synchronizationEnabled && _audio.graph.speedUnit != NULL && queueTime > 0) {
                
                double delay = queueTime - packetStartTime;
                
                if (delay > 0.005)
                    _audio.graph.speedUnit->setPlaybackRate(0.995);
                else if (delay < -0.005)
                    _audio.graph.speedUnit->setPlaybackRate(1.005);
                else if ((delay <= .00025f || delay >= -.00025f) && _audio.graph.speedUnit->getPlaybackRate() != 1.0)
                    _audio.graph.speedUnit->setPlaybackRate(1.0);
                
            }
            
        }
        
    } else if (_outputIsHomed) {
        _outputIsHomed = false;
        log_message(LOG_INFO, "Output lost synchronization");
    }
    
    run++;
    
    mutex_unlock(_timeMutex);
    
    return noErr;
    
}

OSStatus AudioPlayer::_renderCallbackHelper (void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList  *ioData) {
    
    return ((AudioPlayer *)inRefCon)->_renderCallback(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    
}

int AudioPlayer::addAudio(void* buffer, uint32_t size, int seqNo, uint32_t sampleTime) {
    
    mutex_lock(_packetMutex);
    if (seqNo <= _flushedSeq) {
        log_message(LOG_INFO, "Audio packet (seq %d) ignored", seqNo);
        mutex_unlock(_packetMutex);
        return 0;
    } else {
        _flushedSeq = -1;
        _audio.graph.mixerUnit->setVolume(_preFlushVolume, 0);
    }
    mutex_unlock(_packetMutex);
    
    int ret = 0;
    
    uint32_t outsize = _srcFramesPerPacket * _audio.outDesc.mBytesPerFrame;
    void* rawBuffer = malloc(outsize);
    
    _audio.losslessConverter->convert(buffer, size, rawBuffer, &outsize);
    if (outsize == 0)
        log_message(LOG_ERROR, "DECODING ERROR!");
    
    if (size > 0 && _audioQueue->awaitAvailabilty())
        ret = _audioQueue->_addAudioPacket(rawBuffer, outsize, seqNo, sampleTime);
    
    free(rawBuffer);
    
    return ret;
    
}

double AudioPlayer::getSampleRate() {
    
    return _audio.outDesc.mSampleRate;
    
}

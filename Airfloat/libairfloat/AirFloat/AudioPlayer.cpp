//
//  AudioPlayer.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <QuartzCore/QuartzCore.h>

#include "CAHostTimeBase.h"
#include "CAXException.h"
#include "Log.h"

#include "AudioPlayer.h"

#define MIN(x,y) (x < y ? x : y)
#define SILENT_PACKET_SIZE (_srcFramesPerPacket * _audio.outDesc.mBytesPerFrame * 10)
#define MAX_PACKET_SIZE ((double)SILENT_PACKET_SIZE * 1.2)
#define FROM_CLIENT_TIME(x) (x + _clientServerDifference)
#define TO_CLIENT_TIME(x) (x - _clientServerDifference)
#define FROM_DEVICE_TIME(x) (CAHostTimeBase::ConvertToNanos(x) / 1000000000.0)
#define TO_DEVICE_TIME(x) CAHostTimeBase::ConvertFromNanos(x * 1000000000.0)
#define ACCEPTABLE_DELAY 0.01

const char* strspeed(AudioPlayerSpeed speed) {
    
    switch (speed) {
        case kAudioPlayerSpeedNormal:
            return "kAudioPlayerSpeedNormal";
        case kAudioPlayerSpeedSpeedUp:
            return "kAudioPlayerSpeedUp";
        case kAudioPlayerSpeedSlowDown:
            return "kAudioPlayerSlowDown";
    }
    
}

AudioPlayer::AudioPlayer(int* fmts, int fmtsSize) {
    
    assert(fmts != NULL && fmtsSize > 0);
    
    UInt32 category = kAudioSessionCategory_MediaPlayback;
    AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
    AudioSessionSetActive(true);
        
    bzero(&_audio, sizeof(_audio));
    
    _audio.losslessConverter = new AppleLosslessAudioConverter(fmts, fmtsSize);
    _audio.speedUpConverter = new AudioConverter(fmts[10], fmts[10] - 441);
    _audio.slowDownConverter = new AudioConverter(fmts[10], fmts[10] + 441);
    
    _srcFramesPerPacket = fmts[0];
    
    _clientServerDifference = 0;
    _nextPacketTime = 0;
    _speed = kAudioPlayerSpeedNormal;
    _outputIsHomed = false;
    
    bzero(_clientServerDifferenceHistory, sizeof(double) * CLIENT_SERVER_DIFFERENCE_BACKLOG);
    _clientServerDifferenceHistoryCount = 0;
        
    _audio.outDesc = _audio.losslessConverter->GetDestDescription();

    _audioQueue = new AudioQueue(fmts[0], _audio.outDesc.mBytesPerFrame, _audio.outDesc.mSampleRate);
    
    XThrowIfError(
                  AudioQueueNewOutput(&_audio.outDesc, AudioPlayer::_audioQueueCallbackHelper, this, NULL, NULL, 0, &_audio.queue);
                  , "AudioQueueNewOutput");
    
    XThrowIfError(
                  AudioQueueCreateTimeline(_audio.queue, &_audio.timeline)
                  , "AudioQueueCreateTimeline");
    
    for (int i = 0 ; i < BUFFER_COUNT ; i++) {
        AudioQueueAllocateBuffer(_audio.queue, MAX_PACKET_SIZE, &_audio.buffers[i]);
        _audio.buffers[i]->mAudioDataByteSize = MAX_PACKET_SIZE;
        bzero(_audio.buffers[i]->mAudioData, MAX_PACKET_SIZE);
        AudioQueueEnqueueBuffer(_audio.queue, _audio.buffers[i], 0, NULL);
    }
    
    pthread_mutex_init(&_timeMutex, NULL);
    
}

AudioPlayer::~AudioPlayer() {
    
    AudioQueueStop(_audio.queue, true);
    
    for (int i = 0 ; i < BUFFER_COUNT ; i++)
        AudioQueueFreeBuffer(_audio.queue, _audio.buffers[i]);
    
    AudioQueueDispose(_audio.queue, true);
    
    delete _audioQueue;
    
    delete _audio.losslessConverter;
    delete _audio.speedUpConverter;
    delete _audio.slowDownConverter;
    
    pthread_mutex_destroy(&_timeMutex);
    
}

void AudioPlayer::start() {
    
    OSStatus err;
    if (noErr == (err = AudioQueueStart(_audio.queue, NULL)))
        log(LOG_INFO, "Audio queue started.");
    XThrowIfError(
                  err
                  , "AudioQueueStart");
    
}

void AudioPlayer::flush() {
    
    pthread_mutex_lock(&_timeMutex);
    
    _audioQueue->_flush();
    AudioQueueFlush(_audio.queue);
    _outputIsHomed = false;
    _nextPacketTime = 0;
    
    pthread_mutex_unlock(&_timeMutex);
        
}

void AudioPlayer::setVolume(double volume) {
    
    AudioQueueSetParameter(_audio.queue, kAudioQueueParam_Volume, volume);
    
}

void AudioPlayer::setClientTime(double time) {
    
    pthread_mutex_lock(&_timeMutex);
    
    AudioTimeStamp timeStamp;
    bzero(&timeStamp, sizeof(AudioTimeStamp));
    AudioQueueDeviceGetCurrentTime(_audio.queue, &timeStamp);
    
    _clientServerDifference = 0;
    
    _clientServerDifferenceHistoryCount = MIN(_clientServerDifferenceHistoryCount + 1, CLIENT_SERVER_DIFFERENCE_BACKLOG);
    for (int i = _clientServerDifferenceHistoryCount - 1 ; i > 0 ; i--)
        _clientServerDifference += (_clientServerDifferenceHistory[i] = _clientServerDifferenceHistory[i-1]);
    
    _clientServerDifference += (_clientServerDifferenceHistory[0] = CAHostTimeBase::ConvertToNanos(timeStamp.mHostTime) / 1000000000.0 - time);
    
    _clientServerDifference /= (double)_clientServerDifferenceHistoryCount;
    
    log(LOG_INFO, "Time difference: %1.5f", _clientServerDifference);
    
    pthread_mutex_unlock(&_timeMutex);
    
}

AudioQueue* AudioPlayer::getAudioQueue() {
    
    return _audioQueue;
    
}

void AudioPlayer::_audioQueueCallback(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    
    UInt32 size = sizeof(UInt32);
    UInt32 isRunning = 0;
    AudioQueueGetProperty(_audio.queue, kAudioQueueProperty_IsRunning, &isRunning, &size);
    
    if (!isRunning) {
        log(LOG_INFO, "Queue is not running");
        return;
    }
    
    pthread_mutex_lock(&_timeMutex);
    
    inBuffer->mAudioDataByteSize = SILENT_PACKET_SIZE;
    bzero(inBuffer->mAudioData, SILENT_PACKET_SIZE);
    
    double time = 0;
    uint32_t sampleTime = 0;
    
    if (_nextPacketTime > 0 && _audioQueue->hasAvailablePacket()) {
        
        if (!_outputIsHomed) {
            
            time = _audioQueue->getPacketTime();
            
            double timeToStart = (FROM_CLIENT_TIME(time) - _nextPacketTime) + .2;
            inBuffer->mAudioDataByteSize = MIN((timeToStart * _audio.outDesc.mSampleRate * _audio.outDesc.mBytesPerPacket), SILENT_PACKET_SIZE);
            
            if (timeToStart - ((_srcFramesPerPacket * 2) / _audio.outDesc.mSampleRate) <= 0) {
                _outputIsHomed = true;
                log(LOG_INFO, "Output is homed (client time: %1.6f)", TO_CLIENT_TIME(CACurrentMediaTime()));
            }
            
            time = 0;

        } else {
            
            if (_audioQueue->hasAvailablePacket()) {
                
                inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;
                _audioQueue->getPacket(inBuffer->mAudioData, (int*)&inBuffer->mAudioDataByteSize, &time, &sampleTime);
                
                if (_speed == kAudioPlayerSpeedSpeedUp || _speed == kAudioPlayerSpeedSlowDown) {
                    
                    AudioConverter* audioConverter = (_speed == kAudioPlayerSpeedSpeedUp ? _audio.speedUpConverter : _audio.slowDownConverter);
                    
                    int resampledSize = audioConverter->CalculateOutput(inBuffer->mAudioDataByteSize);
                    
                    log(LOG_INFO, "resample");
                    
                    /*
                     int bufferSize = MAX_PACKET_SIZE;
                     char buffer[bufferSize];
                     bzero(buffer, bufferSize);
                     
                     if (_speed == kAudioPlayerSpeedSpeedUp)
                     inBuffer->mAudioDataByteSize -= 70;
                     else
                     inBuffer->mAudioDataByteSize += 70;
                     
                     assert(inBuffer->mAudioDataBytesCapacity >= bufferSize);
                     
                     inBuffer->mAudioDataByteSize = bufferSize;
                     memcpy(inBuffer->mAudioData, buffer, bufferSize);
                     */
                    
                }
                
            }
            
        }
        
    }
    
    AudioTimeStamp timeStamp;
    
    if (noErr == AudioQueueEnqueueBufferWithParameters(_audio.queue, inBuffer, 0, NULL, 0, 0, 0, NULL, NULL, &timeStamp)) {
        
        double currentPacketTime = CAHostTimeBase::ConvertToNanos(timeStamp.mHostTime) / 1000000000.0;
        _nextPacketTime = currentPacketTime + ((inBuffer->mAudioDataByteSize / _audio.outDesc.mBytesPerPacket) / _audio.outDesc.mSampleRate);
        
        if (time > 0 && _outputIsHomed) {
            
            _speed = kAudioPlayerSpeedNormal;

            double audioDelay = (CAHostTimeBase::ConvertToNanos(timeStamp.mHostTime) / 1000000000.0) - (time + _clientServerDifference);
            if (audioDelay > ACCEPTABLE_DELAY)
                _speed = kAudioPlayerSpeedSpeedUp;
            else if (audioDelay < -ACCEPTABLE_DELAY)
                _speed = kAudioPlayerSpeedSlowDown;
            
            log(LOG_INFO, "Audio difference: %1.5f (Speed is %s)", audioDelay, strspeed(_speed));

        }
        
    } else
        log(LOG_ERROR, "Cannot enqueue audio buffer");
    
    pthread_mutex_unlock(&_timeMutex);
    
}

void AudioPlayer::_audioQueueCallbackHelper(void* aqData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    
    assert(aqData != NULL);
    
    ((AudioPlayer*)aqData)->_audioQueueCallback(inAQ, inBuffer);
    
}

int AudioPlayer::addAudio(void* buffer, int size, int seqNo, uint32_t sampleTime) {
    
    int outsize = _srcFramesPerPacket * _audio.outDesc.mBytesPerFrame;
    char rawBuffer[outsize];
    
    _audio.losslessConverter->Convert(buffer, size, rawBuffer, &outsize);
    if (outsize == 0)
        log(LOG_ERROR, "DECODING ERROR!");
    
    return _audioQueue->_addAudioPacket(rawBuffer, outsize, seqNo, sampleTime);
    
}

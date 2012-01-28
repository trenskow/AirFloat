//
//  AudioQueue.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include "Log.h"
#include "AudioQueue.h"

#define QUEUE_THRESHOLD 200
#define UINT16_MIDDLE 0x7fff
#define LoopFrom(x, y, d) for (AudioPacket* x = y ; x != NULL ; x = x->d)
#define MIN(x,y) (x < y ? x : y)
#define MAX(x,y) (x > y ? x : y)

static const char* statestr(AudioPacketState state) {
    
    switch (state) {
        case kAudioPacketStateComplete:
            return "kAudioPacketStateComplete";
        case kAudioPacketStateWaiting:
            return "kAudioPacketStateWaiting";
        case kAudioPacketStateMissing:
            return "kAudioPacketStateMissing";
        case kAudioPacketStateRequested:
            return "kAudioPacketStateRequested";
        default:
            return "kAudioPacketStateUnknown";

    }
    
}

static void dumpqueue(AudioPacket* head) {
    
    assert(head != NULL);
    
    LoopFrom(currentPacket, head, next)
        log(LOG_ERROR, " %05d --> %s", currentPacket->seqNo, statestr(currentPacket->state));
    
}

AudioQueue::AudioQueue(double packetSize, double frameSize, double sampleRate) {
    
    assert(packetSize > 0 && frameSize > 0);
    
    _packetSize = packetSize;
    _frameSize = frameSize;
    _sampleRate = sampleRate;
    
    _foldCount = 0;
    _queueCount = 0;
    _missingCount = 0;
    
    _isSynced = false;
    
    _queueHead = _queueTail = NULL;
    
    _addPacketClbk = NULL;
    _flushClbk = NULL;
    _syncClbk = NULL;
    _addPacketClbkCtx = _flushClbkCtx = _syncClbkCtx = NULL;
        
    pthread_mutex_init(&_mutex, NULL);
    
}

AudioQueue::~AudioQueue() {
    
    pthread_mutex_lock(&_mutex);
    
    while (_queueHead != NULL) {
        AudioPacket* Packet = _queueHead;
        _queueHead = _queueHead->next;
        free(Packet->buffer);
        free(Packet);
    }
    
    pthread_mutex_unlock(&_mutex);
    
    pthread_mutex_destroy(&_mutex);
    
}

void AudioQueue::_checkQueueConsistency() {
    
    int count = 0;
    int missing = 0;
    
    LoopFrom(currentPacket, _queueHead, next) {
        if (currentPacket->state >= kAudioPacketStateMissing)
            missing++;
        count++;
    }
    
    if (count != _queueCount || missing != _missingCount) {
        log(LOG_ERROR, "Queue is currupted!");
        dumpqueue(_queueHead);
        assert(0);
    }
    
}

AudioPacket* AudioQueue::_createPacket(AudioPacketState state) {
    
    AudioPacket* newPacket = (AudioPacket*)malloc(sizeof(AudioPacket));
    bzero(newPacket, sizeof(AudioPacket));
    
    newPacket->bufferSize = _frameSize * _packetSize;
    newPacket->buffer = malloc(newPacket->bufferSize);
    bzero(newPacket->buffer, newPacket->bufferSize);
    
    newPacket->state = state;
    
    _checkQueueConsistency();
    
    return newPacket;
    
}

void AudioQueue::_disposePacket(AudioPacket* packet) {
    
    assert(packet != NULL);
    
    if (packet->buffer != NULL)
        free(packet->buffer);
    
    free(packet);
    
}

void AudioQueue::_addPacketToQueueTail(AudioPacket* packet) {
    
    assert(packet != NULL);
    
    packet->prev = _queueTail;
    
    if (_queueTail != NULL) {
        if (packet->seqNo == 0) {
            packet->seqNo = _queueTail->seqNo + 1;
            // Overflow detection (sequences comes as uint16)
            if (packet->seqNo % UINT16_MAX < _queueTail->seqNo % UINT16_MAX)
                _foldCount++;
        }
        if (packet->sampleTime == 0)
            packet->sampleTime = _queueTail->sampleTime + _packetSize;
        _queueTail->next = packet;
    } else
        _queueHead = packet;
    
    if (packet->state == kAudioPacketStateMissing)
        _missingCount++;
    
    _queueCount++;
    
    _queueTail = packet;
    
    _checkQueueConsistency();
    
}

AudioPacket* AudioQueue::_addEmptyPacket(AudioPacketState state) {
    
    AudioPacket* newPacket = _createPacket(state);
    
    _addPacketToQueueTail(newPacket);
    
    return newPacket;
    
}

AudioPacket* AudioQueue::_replacePacket(AudioPacket* oldPacket, AudioPacket* newPacket) {
    
    assert(oldPacket != NULL && newPacket != NULL);
    
    newPacket->time = oldPacket->time;
    newPacket->seqNo = oldPacket->seqNo;
    newPacket->sampleTime = oldPacket->sampleTime;
    
    if (oldPacket->next != NULL)
        oldPacket->next->prev = newPacket;
    else
        _queueTail = newPacket;
    
    if (oldPacket->prev != NULL)
        oldPacket->prev->next = newPacket;
    else
        _queueHead = newPacket;
    
    newPacket->next = oldPacket->next;
    newPacket->prev = oldPacket->prev;
    
    if (newPacket->state < kAudioPacketStateMissing && oldPacket->state >= kAudioPacketStateMissing)
        _missingCount--;
    else if (newPacket->state >= kAudioPacketStateMissing && oldPacket->state < kAudioPacketStateMissing)
        _missingCount++;
    
    _checkQueueConsistency();

    _disposePacket(oldPacket);
        
    return newPacket;
    
}

AudioPacket* AudioQueue::_popQueueFromHead() {
    
    AudioPacket* headPacket = _queueHead;
    
    if (_queueHead != NULL) {
        _queueHead = _queueHead->next;
        if (_queueHead != NULL)
            _queueHead->prev = NULL;
        else
            _queueTail = NULL;
    }
    
    headPacket->next = NULL;
    headPacket->prev = NULL;
    
    if (headPacket->state >= kAudioPacketStateMissing)
        _missingCount--;
    
    _queueCount--;
    
    _checkQueueConsistency();
    
    return headPacket;
    
}

double AudioQueue::_convertTime(uint32_t fromSampleTime, double fromTime, uint32_t toSampleTime) {
    
    return fromTime + ((toSampleTime - fromSampleTime) / _sampleRate);
    
}

int AudioQueue::_addAudioPacket(void* buffer, int size, int seqNo, uint32_t sampleTime) {
        
    assert(buffer != NULL && size > 0);
    
    int ret = 0;
    
    pthread_mutex_lock(&_mutex);
    
    // Overflow detection
    if (_queueTail != NULL) {
        if (seqNo + UINT16_MIDDLE < _queueTail->seqNo % UINT16_MAX)
            _foldCount++;
        else if (seqNo > (_queueTail->seqNo % UINT16_MAX) + UINT16_MIDDLE)
            seqNo -= UINT16_MAX;
    }
    
    AudioPacket* entryPoint = _queueTail;    
    AudioPacket* newPacket = _createPacket(kAudioPacketStateComplete);
    
    newPacket->sampleTime = sampleTime;
    newPacket->seqNo = (int)seqNo + (_foldCount * UINT16_MAX);
    memcpy(newPacket->buffer, buffer, size);
    
    if (_queueHead == NULL)
        _addPacketToQueueTail(newPacket);
    else if (_queueTail->seqNo < newPacket->seqNo) {
        
        // Add missing packets
        ret = newPacket->seqNo - (_queueTail->seqNo + 1);
        for (int i = 0 ; i < ret ; i++)
            _addEmptyPacket(kAudioPacketStateMissing);
        
        _addPacketToQueueTail(newPacket);
        
    } else {
        
        LoopFrom(currentPacket, _queueTail, prev) {
            
            if (currentPacket->seqNo == newPacket->seqNo) {
                if (currentPacket->state >= kAudioPacketStateMissing)
                    log(LOG_INFO, "Added missing package %d", newPacket->seqNo);
                
                currentPacket = _replacePacket(currentPacket, newPacket);
                entryPoint = currentPacket->prev;
                break;
                
            }
                        
        }
        
    }
    
    if (newPacket != _queueHead && newPacket->prev == NULL && newPacket->next == NULL) {
        log(LOG_INFO, "Packet %d was not added", newPacket->seqNo);
        _disposePacket(newPacket);
    } else if (entryPoint != NULL) {
        
        // Waiting frames inside queue should be set to missing and requested
        // Waiting frames can only trail the last known completed package
        LoopFrom(currentPacket, entryPoint, prev) {
            if (currentPacket->state == kAudioPacketStateWaiting)
                currentPacket = _replacePacket(currentPacket, _createPacket(kAudioPacketStateMissing));
            else
                break;
        }
        
        _checkQueueConsistency();
        
    }
    
    pthread_mutex_unlock(&_mutex);
    
    return MIN(ret, 30);
    
}

bool AudioQueue::hasAvailablePacket() {
    
    pthread_mutex_lock(&_mutex);
    bool ret = (_queueHead != NULL && _isSynced);
    pthread_mutex_unlock(&_mutex);
    
    return ret;
    
}

double AudioQueue::getPacketTime() {
    
    double ret = 0;
    
    pthread_mutex_lock(&_mutex);
    
    if (_queueHead != NULL && _isSynced)
        ret = _queueHead->time;
    
    pthread_mutex_unlock(&_mutex);
    
    return ret;
    
}

void AudioQueue::getPacket(void* buffer, int* size, double* time, uint32_t* sampleTime) {
    
    assert(buffer != NULL && size != NULL);
    
    pthread_mutex_lock(&_mutex);
    
    int outSize = 0;
    
    if (_queueHead != NULL) {
        
        while (_queueHead != NULL && *size - outSize >= _queueHead->bufferSize) {
            
            AudioPacket* audioPacket = _popQueueFromHead();
            
            if (outSize == 0 && sampleTime != NULL)
                *sampleTime = audioPacket->sampleTime;
            if (audioPacket->time > 0 && time != NULL)
                *time = _convertTime(_queueHead->sampleTime, _queueHead->time, *sampleTime);
            
            assert(*size - outSize >= audioPacket->bufferSize);
            memcpy(&((char*)buffer)[outSize], audioPacket->buffer, audioPacket->bufferSize);
            outSize += audioPacket->bufferSize;
            
            if (audioPacket->seqNo % 100 == 0)
                log(LOG_INFO, "%d packages in queue (%d missing / seq %d)", _queueCount, _missingCount, audioPacket->seqNo);
            
            _disposePacket(audioPacket);
            
            /*
            // Fill queue to threshold
            int missingCount = QUEUE_THRESHOLD - _queueCount;
            if (missingCount > 0) {
                for (int missing = 0 ; missing < missingCount ; missing++)
                    _addEmptyPacket(kAudioPacketStateWaiting);
            }
             */
            
        }
        
    }
    
    pthread_mutex_unlock(&_mutex);
    
    *size = outSize;
    
}

void AudioQueue::synchronize(uint32_t currentSampleTime, double currentTime, uint32_t nextSampleTime) {
    
    pthread_mutex_lock(&_mutex);
    
    if (!_isSynced && _queueHead != NULL) {
        _queueHead->time = _convertTime(currentSampleTime, currentTime, _queueHead->sampleTime);
        log(LOG_INFO, "Queue was synced");
        _isSynced = true;
        
        if (_syncClbk != NULL)
            _syncClbk(this, _syncClbkCtx);
        
    }
    
    while (_queueTail != NULL && _queueTail->sampleTime < nextSampleTime)
        _addEmptyPacket(kAudioPacketStateWaiting);
    
    LoopFrom(currentPacket, _queueTail, prev) {
        if (currentPacket->sampleTime <= nextSampleTime && currentPacket->sampleTime + _packetSize > nextSampleTime) {
            currentPacket->time = _convertTime(currentSampleTime, currentTime, nextSampleTime);
            log(LOG_INFO, "Package timestamp set (%1.5f, %1.5f)", currentTime, currentPacket->time);
            pthread_mutex_unlock(&_mutex);
            return;
        }
    }
    
    log(LOG_ERROR, "COULD NOT SET TIMESTAMP");
    
    pthread_mutex_unlock(&_mutex);
    
}

int AudioQueue::getNextMissingWindow(int* seqNo) {
    
    assert(seqNo != NULL);
    
    int ret = 0;
    int count = 0;
    
    pthread_mutex_lock(&_mutex);
    
    LoopFrom(currentPacket, _queueHead, next) {
        
        if (currentPacket->state == kAudioPacketStateMissing) {
            *seqNo = currentPacket->seqNo % UINT16_MAX;
            
            LoopFrom(missingPacket, currentPacket, next) {
                if (missingPacket->state == kAudioPacketStateMissing) {
                    missingPacket->state = kAudioPacketStateRequested;
                    ret++;
                    count++;
                } else
                    break;
            }
            
        }

        count++;
        
        if (ret > 0 || count >= _queueCount - 100)
            break;
        
    }
    
    pthread_mutex_unlock(&_mutex);
    
    return MIN(ret, 30);
    
}

void AudioQueue::setAddPacketCallback(audioQueueAddPacketClbk clbk, void* ctx) {
    
    _addPacketClbk = clbk;
    _addPacketClbkCtx = ctx;
    
}

void AudioQueue::setFlushCallback(audioQueueSimpleClbk clbk, void* ctx) {
    
    _flushClbk = clbk;
    _flushClbkCtx = ctx;
    
}

void AudioQueue::setSyncCallback(audioQueueSimpleClbk clbk, void* ctx) {
    
    _syncClbk = clbk;
    _syncClbkCtx = ctx;
    
}

void AudioQueue::_flush() {
    
    pthread_mutex_lock(&_mutex);
    
    while (_queueHead != NULL)
        _disposePacket(_popQueueFromHead());
    
    _queueTail = NULL;
    
    _isSynced = false;
    
    log(LOG_INFO, "Queue flushed");
    
    if (_flushClbk != NULL)
        _flushClbk(this, _flushClbkCtx);
    
    pthread_mutex_unlock(&_mutex);
    
}

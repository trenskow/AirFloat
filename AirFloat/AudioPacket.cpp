//
//  AudioPacket.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/4/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include "AudioPacket.h"

#define MIN(x, y) (x < y ? x : y)

AudioPacket::AudioPacket(AudioPacketState initialState) {
    
    seqNo = 0;
    sampleTime = 0;
    state = initialState;

    _bufferSize = 0;
    _buffer = NULL;
    
    next = prev = NULL;

}

AudioPacket::~AudioPacket() {
    
    setBuffer(NULL, 0);
    
}

void AudioPacket::setBuffer(void* buffer, uint32_t size) {
    
    _mutex.lock();
    
    if (_buffer != NULL)
        free(_buffer);
    _bufferSize = 0;
    
    if (buffer != NULL) {
        _buffer = malloc(size);
        memcpy(_buffer, buffer, size);
        _bufferSize = size;
    } else
        _buffer = NULL;
    
    _mutex.unlock();
    
}

uint32_t AudioPacket::getBuffer(void* buffer, uint32_t size) {
    
    uint32_t ret = 0;
    
    _mutex.lock();
    
    ret = MIN(size, _bufferSize);
    if (_buffer != NULL && buffer != NULL)
        memcpy(buffer, _buffer, ret);
    
    _mutex.unlock();
    
    return ret;
    
}


bool AudioPacket::hasBuffer() {
    
    _mutex.lock();
    
    bool ret = (_buffer != NULL && _bufferSize > 0);
    
    _mutex.unlock();
    
    return ret;
    
}

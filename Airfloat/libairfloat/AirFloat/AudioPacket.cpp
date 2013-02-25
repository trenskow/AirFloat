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
    time = 0;
    state = initialState;

    _bufferSize = 0;
    _buffer = NULL;
    
    next = prev = NULL;
    
    _mutex = mutex_create();

}

AudioPacket::~AudioPacket() {
    
    setBuffer(NULL, 0);
    
    mutex_destroy(_mutex);
    
}

void AudioPacket::setBuffer(void* buffer, uint32_t size) {
    
    mutex_lock(_mutex);
    
    if (_buffer != NULL)
        free(_buffer);
    _bufferSize = 0;
    
    if (buffer != NULL) {
        _buffer = malloc(size);
        memcpy(_buffer, buffer, size);
        _bufferSize = size;
    } else
        _buffer = NULL;
    
    mutex_unlock(_mutex);
    
}

uint32_t AudioPacket::getBuffer(void* buffer, uint32_t size) {
    
    uint32_t ret = 0;
    
    mutex_lock(_mutex);
    
    ret = MIN(size, _bufferSize);
    if (_buffer != NULL && buffer != NULL)
        memcpy(buffer, _buffer, ret);
    
    mutex_unlock(_mutex);
    
    return ret;
    
}

void AudioPacket::shiftBuffer(uint32_t size) {
    
    mutex_lock(_mutex);
    
    if (size >= _bufferSize) {
        
        if (_buffer != NULL)
            free(_buffer);
        _buffer = NULL;
        _bufferSize = 0;
        
    } else {
        
        memcpy(_buffer, &((char*)_buffer)[size], _bufferSize - size);
        _bufferSize -= size;
        
    }
    
    mutex_unlock(_mutex);
    
}

uint32_t AudioPacket::getBufferSize() {
    
    mutex_lock(_mutex);
    
    bool ret = (_buffer != NULL && _bufferSize > 0);
    
    mutex_unlock(_mutex);
    
    return ret;
    
}

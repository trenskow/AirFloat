//
//  AudioPacket.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/4/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AudioPacket_h
#define AirFloat_AudioPacket_h

#include <stdint.h>

extern "C" {
#include "mutex.h"
}

typedef enum {
    
    kAudioPacketStateComplete = 0,
    kAudioPacketStateMissing,
    kAudioPacketStateRequested
    
} AudioPacketState;

class AudioPacket {

public:
    
    AudioPacket(AudioPacketState initialState = kAudioPacketStateMissing);
    ~AudioPacket();
    
    void setBuffer(void* buffer, uint32_t size);
    uint32_t getBuffer(void* buffer, uint32_t size);
    void shiftBuffer(uint32_t size);
    uint32_t getBufferSize();
    
    void remove();
    
    int seqNo;
    uint32_t sampleTime;
    double time;
    AudioPacketState state;
    
    AudioPacket* next;
    AudioPacket* prev;
    
private:
    
    void* _buffer;
    uint32_t _bufferSize;
    
    mutex_p _mutex;
    
};

#endif

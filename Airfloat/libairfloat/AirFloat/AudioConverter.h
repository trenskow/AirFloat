//
//  AudioConverter.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AudioConverter_h
#define AirFloat_AudioConverter_h

#include "Mutex.h"
#include <AudioToolbox/AudioToolbox.h>

class AudioConverter {
    
public:
    AudioConverter(uint32_t channels, double sampleRate, uint32_t bitDepth);
    virtual ~AudioConverter();
    
    virtual void convert(void* srcBuffer, uint32_t srcSize, void* destBuffer, uint32_t* destSize);
    virtual int calculateOutput(int inputSize);
    virtual AudioStreamBasicDescription getDestDescription();
    
protected:
    
    static OSStatus _audioConverterComplexInputDataProc (AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription  **outDataPacketDescription, void *inUserData);
    
    AudioConverterRef _converter;
    AudioStreamBasicDescription _srcDesc;
    AudioStreamBasicDescription _destDesc;
    AudioBuffer _buffer;
    AudioStreamPacketDescription _pckDesc;
    void* _currentBuffer;
    int _currentBufferSize;
    
    Mutex _decoderMutex;
    
private:
    
    void _setupAudioDescription(AudioStreamBasicDescription* desc, uint32_t channels, double sampleRate, uint32_t bitDepth);
        
};

#endif

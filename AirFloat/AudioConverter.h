//
//  AudioConverter.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AudioConverter_h
#define AirFloat_AudioConverter_h

#include <AudioToolbox/AudioToolbox.h>

class AudioConverter {
    
public:
    AudioConverter(int srcSampleRate, int destSampleRate);
    virtual ~AudioConverter();
    
    virtual void Convert(void* srcBuffer, int srcSize, void* destBuffer, int* destSize);
    int CalculateOutput(int inputSize);
    AudioStreamBasicDescription GetDestDescription();
    
protected:
    
    static OSStatus _audioConverterComplexInputDataProc (AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription  **outDataPacketDescription, void *inUserData);
    
    AudioConverterRef _converter;
    AudioStreamBasicDescription _srcDesc;
    AudioStreamBasicDescription _destDesc;
    AudioBuffer _buffer;
    AudioStreamPacketDescription _pckDesc;
    void* _currentBuffer;
    int _currentBufferSize;
    
private:
    
    void _setupAudioDescription(AudioStreamBasicDescription* desc, int sampleRate);
        
};

#endif

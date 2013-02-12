//
//  AppleLosslessAudioConverter.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AppleLosslessAudioConverter_h
#define AirFloat_AppleLosslessAudioConverter_h

#include "AudioConverter.h"

class AppleLosslessAudioConverter : public AudioConverter {
    
public:
    AppleLosslessAudioConverter(int* fmts, int fmtsSize);
    
    virtual void convert(void* srcBuffer, uint32_t srcSize, void* destBuffer, uint32_t* destSize);

protected:
    static OSStatus _audioConverterComplexInputDataProc (AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription  **outDataPacketDescription, void *inUserData);
    
};


#endif

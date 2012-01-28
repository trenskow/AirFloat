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
    
    virtual void Convert(void* srcBuffer, int srcSize, void* destBuffer, int* destSize);

protected:
    static OSStatus _audioConverterComplexInputDataProc (AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription  **outDataPacketDescription, void *inUserData);
    
};


#endif

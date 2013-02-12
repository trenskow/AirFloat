//
//  AppleLosslessSoftwareConverter.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/30/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_AppleLosslessSoftwareConverter_h
#define AirFloat_AppleLosslessSoftwareConverter_h

extern "C" {
#include "alac.h"
}

#include "AudioConverter.h"

class AppleLosslessSoftwareAudioConverter : public AudioConverter {
    
public:
    AppleLosslessSoftwareAudioConverter(int* fmts, int fmtsSize);
    virtual ~AppleLosslessSoftwareAudioConverter();
    
    virtual void convert(void* srcBuffer, uint32_t srcSize, void* destBuffer, uint32_t* destSize);
    int calculateOutput(int inputSize);
    AudioStreamBasicDescription getDestDescription();

private:
    
    alac_file* _decoderInfo;
    
};

#endif

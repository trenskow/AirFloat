//
//  AppleLosslessSoftwareConverter.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 1/30/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

extern "C" {
#include "log.h"
}

#include "AppleLosslessSoftwareAudioConverter.h"

AppleLosslessSoftwareAudioConverter::AppleLosslessSoftwareAudioConverter(int* fmts, int fmtsSize) : AudioConverter(fmts[6], fmts[10], fmts[2]) {
    
    int frame_size = fmts[0];
    int sample_size = fmts[2];

    _decoderInfo = create_alac(sample_size, 2);
    
    _decoderInfo->setinfo_max_samples_per_frame = frame_size;
    _decoderInfo->setinfo_7a =      fmts[1];
    _decoderInfo->setinfo_sample_size = sample_size;
    _decoderInfo->setinfo_rice_historymult = fmts[3];
    _decoderInfo->setinfo_rice_initialhistory = fmts[4];
    _decoderInfo->setinfo_rice_kmodifier = fmts[5];
    _decoderInfo->setinfo_7f =      fmts[6];
    _decoderInfo->setinfo_80 =      fmts[7];
    _decoderInfo->setinfo_82 =      fmts[8];
    _decoderInfo->setinfo_86 =      fmts[9];
    _decoderInfo->setinfo_8a_rate = fmts[10];
    
    allocate_buffers(_decoderInfo);
    
}

AppleLosslessSoftwareAudioConverter::~AppleLosslessSoftwareAudioConverter() {
    
    deallocate_buffers(_decoderInfo);
    dispose_alac(_decoderInfo);
    
}

void AppleLosslessSoftwareAudioConverter::convert(void* srcBuffer, uint32_t srcSize, void* destBuffer, uint32_t* destSize) {
    
    mutex_lock(_decoderMutex);
    decode_frame(_decoderInfo, (unsigned char*)srcBuffer, (unsigned char*)destBuffer, (int*)destSize);
    mutex_unlock(_decoderMutex);
    
}

int AppleLosslessSoftwareAudioConverter::calculateOutput(int inputSize) {
    
    return 0;
    
}

AudioStreamBasicDescription AppleLosslessSoftwareAudioConverter::getDestDescription() {
    
    AudioStreamBasicDescription outDesc = AudioConverter::getDestDescription();
    
    outDesc.mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    outDesc.mBytesPerFrame = (outDesc.mChannelsPerFrame * (outDesc.mBitsPerChannel / 8));
    outDesc.mBytesPerPacket = outDesc.mBytesPerFrame * outDesc.mFramesPerPacket;
    
    UInt32 size = sizeof(outDesc);
    OSStatus err = AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &outDesc);
    if (err != noErr)
        log_message(LOG_ERROR, "Error");
    
    return outDesc;
    
}
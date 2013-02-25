//
//  AppleLosslessAudioConverter.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

extern "C" {
#include "log.h"
}

#include "CAXException.h"
#include "AppleLosslessAudioConverter.h"

static int isBigEndian()
{
    
    int no = 1;
    char *chk = (char *)&no;
    
    if (chk[0] == 1)
        return 0;
    
    return 1;
    
}

static uint32_t bil(uint32_t num) {
    
    if (isBigEndian())
        return num;
    
    return ((num>>24)&0xff) | // move byte 3 to byte 0
    ((num<<8)&0xff0000) | // move byte 1 to byte 2
    ((num>>8)&0xff00) | // move byte 2 to byte 1
    ((num<<24)&0xff000000);
    
}

static uint16_t bis(uint16_t num) {
    
    if (isBigEndian())
        return num;
    
    return ((num >> 8)&0xFF) |
    ((num << 8)&0xFF00);
    
}

typedef struct ALACSpecificConfig {
    uint32_t        frameLength;
    uint8_t         compatibleVersion;
    uint8_t         bitDepth;
    uint8_t         pb;
    uint8_t         mb;
    uint8_t         kb;
    uint8_t         numChannels;
    uint16_t        maxRun;
    uint32_t        maxFrameBytes;
    uint32_t        avgBitRate;
    uint32_t        sampleRate;
} ALACSpecificConfig; 

typedef struct ALACMagicCookie {
    
    struct {
        uint32_t atomSize;
        uint32_t channelLayoutInfoId;
        uint32_t type;
    } formatAtom;
    struct {
        uint32_t infoSize;
        uint32_t id;
        uint32_t versionFlag;
        ALACSpecificConfig config;
    } ALACSpecificInfo;
    struct {        
        uint32_t channelLayoutInfoSize;
        uint32_t channelLayoutInfoId;
    } terminatorAtom;
    
} ALACMagicCookie;

AppleLosslessAudioConverter::AppleLosslessAudioConverter(int* fmts, int fmtsSize) : AudioConverter(fmts[6], fmts[10], fmts[2]) {
    
    bzero(&_srcDesc, sizeof(AudioStreamBasicDescription));
    
    _srcDesc.mFormatID = kAudioFormatAppleLossless;
    _srcDesc.mSampleRate = fmts[10];
    _srcDesc.mFramesPerPacket = fmts[0];
    _srcDesc.mChannelsPerFrame = fmts[6];
    
    ALACSpecificConfig config = { bil(fmts[0]), fmts[1], fmts[2], fmts[3], fmts[4], fmts[5], fmts[6], bis(fmts[7]), bil(fmts[8]), bil(fmts[9]), bil(fmts[10]) };
    
    ALACMagicCookie cookie;
    cookie.formatAtom.atomSize = bil(12);
    cookie.formatAtom.channelLayoutInfoId = bil('frma'),
    cookie.formatAtom.type = bil('alac');
    cookie.ALACSpecificInfo.infoSize = bil(36);
    cookie.ALACSpecificInfo.id = bil('alac');
    cookie.ALACSpecificInfo.versionFlag = 0;
    cookie.ALACSpecificInfo.config = config;
    cookie.terminatorAtom.channelLayoutInfoSize = bil(8);
    cookie.terminatorAtom.channelLayoutInfoId = 0;
    
    OSStatus err = AudioConverterNew(&_srcDesc, &_destDesc, &_converter);
    XThrowIfError(err, "AudioConverterNew");
    
    AudioChannelLayout layout;
    bzero(&layout, sizeof(layout));
    layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
    
    AudioConverterSetProperty(_converter, kAudioConverterDecompressionMagicCookie, sizeof(cookie), &cookie);
    AudioConverterSetProperty(_converter, kAudioConverterInputChannelLayout, sizeof(layout), &layout);
    
}

void AppleLosslessAudioConverter::convert(void* srcBuffer, uint32_t srcSize, void* destBuffer, uint32_t* destSize) {
    
    assert(srcBuffer != NULL && srcSize > 0 && destBuffer != NULL && destSize != NULL && *destSize > 0);
    
    mutex_lock(_decoderMutex);
    
    UInt32 outSize = _destDesc.mBytesPerFrame * _srcDesc.mFramesPerPacket;

    _buffer.mData = srcBuffer;
    _buffer.mDataByteSize = srcSize;
    _buffer.mNumberChannels = 2;
    
    AudioBufferList outBufferList;
    outBufferList.mNumberBuffers = 1;
    outBufferList.mBuffers[0].mData = destBuffer;
    outBufferList.mBuffers[0].mDataByteSize = *destSize;
    
    _pckDesc.mStartOffset = 0;
    _pckDesc.mVariableFramesInPacket = 1;
    _pckDesc.mDataByteSize = srcSize;
    
    _currentBufferSize = srcSize;
    _currentBuffer = srcBuffer;
    
    UInt32 ioOutputDataPackets = outSize / _destDesc.mBytesPerPacket;
    OSStatus err = AudioConverterFillComplexBuffer(_converter, AudioConverter::_audioConverterComplexInputDataProc, this, &ioOutputDataPackets, &outBufferList, NULL);
    *destSize = (err == noErr ? _srcDesc.mFramesPerPacket * _destDesc.mBytesPerFrame : 0);
    
    mutex_unlock(_decoderMutex);
    
}

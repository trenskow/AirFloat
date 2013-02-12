//
//  AudioConverter.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include "Log.h"
#include "AudioConverter.h"

AudioConverter::AudioConverter(double srcSampleRate, double destSampleRate) {
    
    assert(srcSampleRate > 0 && destSampleRate > 0);
    
    _setupAudioDescription(&_srcDesc, srcSampleRate);
    _setupAudioDescription(&_destDesc, destSampleRate);
    
    bzero(&_pckDesc, sizeof(_pckDesc));
    
    _converter = NULL;
    
}

AudioConverter::~AudioConverter() {
    
    if (_converter != NULL)
        AudioConverterDispose(_converter);
    
}

AudioStreamBasicDescription AudioConverter::getDestDescription() {
    
    return _destDesc;
    
}

void AudioConverter::_setupAudioDescription(AudioStreamBasicDescription* desc, double sampleRate) {
    
    assert(desc != NULL && sampleRate > 0);
    
    bzero(desc, sizeof(AudioStreamBasicDescription));
    desc->mFormatID = kAudioFormatLinearPCM;
    desc->mFormatFlags = kAudioFormatFlagIsFloat;
    desc->mSampleRate = sampleRate;
    desc->mChannelsPerFrame = 2;
    desc->mFramesPerPacket = 1;
    desc->mBitsPerChannel = sizeof(float) * 8;
    desc->mBytesPerFrame = desc->mChannelsPerFrame * (desc->mBitsPerChannel / 8);
    desc->mBytesPerPacket = desc->mBytesPerFrame;
    
}

OSStatus AudioConverter::_audioConverterComplexInputDataProc (AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription  **outDataPacketDescription, void *inUserData) {
    
    AudioConverter* converter = (AudioConverter*)inUserData;
    
    if (ioNumberDataPackets)
        *ioNumberDataPackets = (converter->_srcDesc.mFormatID == kAudioFormatLinearPCM ? converter->_currentBufferSize / converter->_srcDesc.mBytesPerFrame : 1);
    (*ioData).mBuffers[0].mData = converter->_currentBuffer;
    (*ioData).mBuffers[0].mDataByteSize = converter->_currentBufferSize;
    (*ioData).mBuffers[0].mNumberChannels = 2;
    
    if (outDataPacketDescription)
        *outDataPacketDescription = &converter->_pckDesc;
    
    return 0;
    
}

void AudioConverter::convert(void* srcBuffer, uint32_t srcSize, void* destBuffer, uint32_t* destSize) {
    
    assert(srcBuffer != NULL && srcSize > 0 && destBuffer != NULL && destSize != NULL && *destSize > 0);
    
    _decoderMutex.lock();
    
    if (!_converter) {
        if (noErr != AudioConverterNew(&_srcDesc, &_destDesc, &_converter))
            throw "Cannot create converter";
        else if (_srcDesc.mSampleRate != _destDesc.mSampleRate) {
            int inValue = kAudioConverterSampleRateConverterComplexity_Mastering;
            AudioConverterSetProperty(_converter, kAudioConverterSampleRateConverterComplexity, sizeof(int), &inValue);
            inValue = kAudioConverterQuality_Max;
            AudioConverterSetProperty(_converter, kAudioConverterSampleRateConverterQuality, sizeof(int), &inValue);
        }
    }
    
    _currentBuffer = srcBuffer;
    _currentBufferSize = srcSize;
    
    _buffer.mData = srcBuffer;
    _buffer.mDataByteSize = srcSize;
    _buffer.mNumberChannels = 2;
    
    AudioBufferList outBufferList;
    outBufferList.mNumberBuffers = 1;
    outBufferList.mBuffers[0].mData = destBuffer;
    outBufferList.mBuffers[0].mDataByteSize = *destSize;
    outBufferList.mBuffers[0].mNumberChannels = 2;
    
    UInt32 ioOutputDataPackets = srcSize / _srcDesc.mBytesPerPacket;
    
    if (noErr != AudioConverterFillComplexBuffer(_converter, AudioConverter::_audioConverterComplexInputDataProc, this, &ioOutputDataPackets, &outBufferList, NULL))
        log(LOG_ERROR, "Unable to convert sample rate");
    else {
        
        *destSize = ioOutputDataPackets * _destDesc.mBytesPerFrame;
        
        bool equal = true;
        if (*destSize != srcSize)
            equal = false;
        else
            for (int i = 0 ; i < srcSize ; i++)
                equal = equal && (((char*)srcBuffer)[i] == ((char*)destBuffer)[i]);
        
        if (equal)
            log(LOG_ERROR, "Converter did not change packet");
    }
    
    _decoderMutex.unlock();
    
}

int AudioConverter::calculateOutput(int inputSize) {
    
    UInt32 size = sizeof(UInt32);
    UInt32 inSize = inputSize;
    
    AudioConverterGetProperty(_converter, kAudioConverterPropertyCalculateOutputBufferSize, &size, &inSize);
    
    return inSize;
    
}

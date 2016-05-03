//
//  AudioPlayer.cpp
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifdef __APPLE__

#import <stdint.h>
#import <stdbool.h>

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif
#import <AVFoundation/AVFoundation.h>

#import "log.h"
#import "audiooutput.h"

#define ca_assert(error) assert((error) == noErr)

double hardware_host_time_to_seconds(double host_time);

typedef void (*audio_output_callback)(audio_output_p ao, void* buffer, size_t size, double host_time, void* ctx);

struct audio_output_t {
    AUGraph graph;
    AudioUnit converter_unit;
    AudioUnit mixer_unit;
    bool has_speed_control;
    AudioUnit speed_unit;
    AudioUnit output_unit;
    audio_output_callback callback;
    void* callback_ctx;
};

void audio_output_stop(struct audio_output_t* ao);

AudioUnit _audio_output_create_add_unit(struct audio_output_t* ao, OSType type, OSType subtype, OSType manufacturer, AUNode* node) {
    
    AudioComponentDescription desc;
    bzero(&desc, sizeof(AudioComponentDescription));
    desc.componentType = type;
    desc.componentSubType = subtype;
    desc.componentManufacturer = manufacturer;
    
    AudioUnit unit;
    UInt32 maximumSlicesPerFrame = 4096;
    ca_assert(AUGraphAddNode(ao->graph, &desc, node));
    ca_assert(AUGraphNodeInfo(ao->graph, *node, NULL, &unit));
    ca_assert(AudioUnitSetProperty(unit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &maximumSlicesPerFrame, sizeof(UInt32)));
    
    return unit;
    
}

void _audio_output_connect_unit(struct audio_output_t* ao, AUNode output_node, UInt32 output_node_bus, AUNode input_node, UInt32 input_node_bus) {
    
    AudioUnit output_unit, input_unit;
    ca_assert(AUGraphNodeInfo(ao->graph, output_node, NULL, &output_unit));
    ca_assert(AUGraphNodeInfo(ao->graph, input_node, NULL, &input_unit));
    
    AudioStreamBasicDescription out_desc;
    UInt32 size = sizeof(AudioStreamBasicDescription);
    ca_assert(AudioUnitGetProperty(output_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, output_node_bus, &out_desc, &size));
    if (noErr != AudioUnitSetProperty(input_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, input_node_bus, &out_desc, sizeof(AudioStreamBasicDescription))) {
        
        AudioStreamBasicDescription in_desc;
        size = sizeof(AudioStreamBasicDescription);
        ca_assert(AudioUnitGetProperty(input_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, input_node_bus, &in_desc, &size));
        
        AUNode converter_node;
        AudioUnit converter_unit = _audio_output_create_add_unit(ao, kAudioUnitType_FormatConverter, kAudioUnitSubType_AUConverter, kAudioUnitManufacturer_Apple, &converter_node);
        ca_assert(AudioUnitSetProperty(converter_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &out_desc, sizeof(AudioStreamBasicDescription)));
        ca_assert(AudioUnitSetProperty(converter_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &in_desc, sizeof(AudioStreamBasicDescription)));
        ca_assert(AUGraphConnectNodeInput(ao->graph, output_node, output_node_bus, converter_node, 0));
        ca_assert(AUGraphConnectNodeInput(ao->graph, converter_node, 0, input_node, input_node_bus));
        
    } else
        ca_assert(AUGraphConnectNodeInput(ao->graph, output_node, output_node_bus, input_node, input_node_bus));
    
}

OSStatus _audio_unit_render_callback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) {
    
    struct audio_output_t* ao = (struct audio_output_t*)inRefCon;
    
    bzero(ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize);
    
    ca_assert(AudioUnitSetParameter(ao->mixer_unit, kMultiChannelMixerParam_Enable, kAudioUnitScope_Output, 0, 1.0, 0));
    
    if (ao->callback)
        ao->callback(ao, ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize, hardware_host_time_to_seconds(inTimeStamp->mHostTime), ao->callback_ctx);
    
    return noErr;
    
}

struct audio_output_t* audio_output_create(struct decoder_output_format_t decoder_output_format) {
    
    struct audio_output_t* ao = (struct audio_output_t*)malloc(sizeof(struct audio_output_t));
    bzero(ao, sizeof(struct audio_output_t));
    
    ca_assert(NewAUGraph(&ao->graph));
    ca_assert(AUGraphOpen(ao->graph));
    
    AudioStreamBasicDescription in_desc;
    bzero(&in_desc, sizeof(AudioStreamBasicDescription));
    in_desc.mFormatID = kAudioFormatLinearPCM;
    in_desc.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    in_desc.mSampleRate = decoder_output_format.sample_rate;
    in_desc.mChannelsPerFrame = decoder_output_format.channels;
    in_desc.mBitsPerChannel = decoder_output_format.bit_depth;
    in_desc.mFramesPerPacket = 1;
    
    UInt32 size = sizeof(AudioStreamBasicDescription);
    ca_assert(AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &in_desc));
    
    AUNode mixer_node;
    ao->mixer_unit = _audio_output_create_add_unit(ao, kAudioUnitType_Mixer, kAudioUnitSubType_MultiChannelMixer, kAudioUnitManufacturer_Apple, &mixer_node);
    
#if TARGET_OS_MAC
    ca_assert(AudioUnitSetParameter(ao->mixer_unit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, 1.0, 0));
#endif
    
    AUNode input_node = mixer_node;
    
    OSStatus err = AudioUnitSetProperty(ao->mixer_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &in_desc, sizeof(AudioStreamBasicDescription));
    if (err != noErr) {
        
        AUNode converter_node;
        ao->converter_unit = _audio_output_create_add_unit(ao, kAudioUnitType_FormatConverter, kAudioUnitSubType_AUConverter, kAudioUnitManufacturer_Apple, &converter_node);
        
        ca_assert(AudioUnitSetProperty(ao->converter_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &in_desc, sizeof(AudioStreamBasicDescription)));
        AudioStreamBasicDescription mixer_output_desc;
        size = sizeof(AudioStreamBasicDescription);
        ca_assert(AudioUnitGetProperty(ao->mixer_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &mixer_output_desc, &size));
        ca_assert(AudioUnitSetProperty(ao->converter_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &mixer_output_desc, size));
        _audio_output_connect_unit(ao, converter_node, 0, mixer_node, 0);
        
        input_node = converter_node;
        
    }
    
    AURenderCallbackStruct render_callback;
    render_callback.inputProc = _audio_unit_render_callback;
    render_callback.inputProcRefCon = ao;
    
    ca_assert(AUGraphSetNodeInputCallback(ao->graph, input_node, 0, &render_callback));
    
    AUNode output_node;
#if TARGET_OS_IPHONE
    ao->output_unit = _audio_output_create_add_unit(ao, kAudioUnitType_Output, kAudioUnitSubType_RemoteIO, kAudioUnitManufacturer_Apple, &output_node);
#else
    ao->output_unit = _audio_output_create_add_unit(ao, kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, &output_node);
#endif
    
    double use_speed = true;
#if TARGET_OS_IPHONE
    @autoreleasepool {
        use_speed = [[UIDevice currentDevice].systemVersion floatValue] >= 6;
    }
#endif
    
    if (use_speed) { // Darwin 11 is iOS 5. Varispeed is only available in iOS 5+.
        
        ao->has_speed_control = true;
        AUNode speed_node;
        ao->speed_unit = _audio_output_create_add_unit(ao, kAudioUnitType_FormatConverter, kAudioUnitSubType_Varispeed, kAudioUnitManufacturer_Apple, &speed_node);
        _audio_output_connect_unit(ao, mixer_node, 0, speed_node, 0);
        
        _audio_output_connect_unit(ao, speed_node, 0, output_node, 0);
        
    } else
        _audio_output_connect_unit(ao, mixer_node, 0, output_node, 0);
    
    ca_assert(AUGraphInitialize(ao->graph));
    ca_assert(AUGraphUpdate(ao->graph, NULL));
    
    return ao;
    
}

void audio_output_destroy(struct audio_output_t* ao) {
    
    audio_output_stop(ao);
    
    ca_assert(AUGraphUninitialize(ao->graph));
    ca_assert(DisposeAUGraph(ao->graph));
    
    free(ao);
    
}

void audio_output_set_callback(struct audio_output_t* ao, audio_output_callback callback, void* ctx) {
    
    ao->callback = callback;
    ao->callback_ctx = ctx;
    
}

void audio_output_session_start () {
    
        double sampleRate = 44100.0;
        float frameCount = 4096.0f;
        double bufferLength = (frameCount/sampleRate);   // 1500ms
    
        // AVAudioSession: Sample Rate
        NSError *rateError = nil;
        [[AVAudioSession sharedInstance] setPreferredSampleRate:sampleRate error:&rateError];
        if (rateError) {
            log_message(LOG_ERROR, "Error setting SampleRate: %@", [rateError description]);
        }
        
        // AVAudioSession: Buffer Duration
        NSError *bufferError = nil;
        [[AVAudioSession sharedInstance] setPreferredIOBufferDuration:bufferLength error:&bufferError];
        if (bufferError) {
            log_message(LOG_ERROR, "Error setting BufferDuration: %@", [bufferError description]);
        }
        
        // AVAudioSession: Category
        NSError *categoryError = nil;
        [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryPlayback error:&categoryError];
        if (categoryError) {
            log_message(LOG_ERROR, "Error setting Category: %@", [categoryError description]);
        }
        
        // AVAudioSession: Activation
        NSError *activationError = nil;
        BOOL didActivate = [[AVAudioSession sharedInstance] setActive: YES error: &activationError];
        if (!didActivate) {
            if (activationError) {
                log_message(LOG_ERROR, "Could not activate AVAudioSession, Error %@", [activationError localizedDescription]);
            } else {
                log_message(LOG_ERROR, "Could not activate AVAudioSession");
            }
        }
}

void audio_output_session_stop () {
    
    // AVAudioSession: De-Activation
    NSError *deactivationError = nil;
    BOOL didDeactivate = [[AVAudioSession sharedInstance] setActive:NO withOptions: AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation error: &deactivationError];
    if (!didDeactivate) {
        if (deactivationError) {
            log_message(LOG_ERROR, "Could not deactivate AVAudioSession, Error %@", [deactivationError localizedDescription]);
        } else {
            log_message(LOG_ERROR, "Could not deactivate AVAudioSession");
        }
    }
}

void audio_output_start(struct audio_output_t* ao) {
    
#if TARGET_OS_IPHONE
    @autoreleasepool {
        audio_output_session_start();
    }
#endif
    
    ca_assert(AUGraphStart(ao->graph));
    
}

void audio_output_stop(struct audio_output_t* ao) {
    
    ca_assert(AUGraphStop(ao->graph));
    
#if TARGET_OS_IPHONE
    @autoreleasepool {
        audio_output_session_stop();
    }
#endif
    
}

void audio_output_flush(struct audio_output_t* ao) {
    
    ca_assert(AudioUnitSetParameter(ao->mixer_unit, kMultiChannelMixerParam_Enable, kAudioUnitScope_Output, 0, 0.0, 0));
    
}

double audio_output_get_playback_rate(audio_output_p ao) {
    
    AudioUnitParameterValue value = 1.0;
    
    if (ao->has_speed_control) {
        ca_assert(AudioUnitGetParameter(ao->speed_unit, kVarispeedParam_PlaybackRate, kAudioUnitScope_Global, 0, &value));
    }
    
    return value;
    
}

void audio_output_set_playback_rate(audio_output_p ao, double playback_rate) {
    
    if (ao->has_speed_control) {
        AudioUnitParameterValue value = playback_rate;
        ca_assert(AudioUnitSetParameter(ao->speed_unit, kVarispeedParam_PlaybackRate, kAudioUnitScope_Global, 0, value, 0));
    }
    
}

void audio_output_set_volume(struct audio_output_t* ao, double volume) {
    
    ca_assert(AudioUnitSetParameter(ao->mixer_unit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, volume, 0));
    
}

void audio_output_set_muted(struct audio_output_t* ao, bool muted) {
    
    ca_assert(AudioUnitSetParameter(ao->mixer_unit, kMultiChannelMixerParam_Enable, kAudioUnitScope_Input, 0, (muted ? 0.0 : 1.0), 0));
    
}

#endif

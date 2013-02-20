//
//  AudioMixerUnit.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#include "CAXException.h"
#include "AudioUnitMixer.h"

using namespace Audio;

UnitMixer::UnitMixer(uint32_t busCount, Graph *inGraph) : Unit(kAudioUnitType_Mixer, kAudioUnitSubType_MultiChannelMixer, kAudioUnitManufacturer_Apple, inGraph) {
    
    XThrowIfError(AudioUnitSetProperty(_unit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &busCount, sizeof(busCount)), "AudioUnitSetProperty(kAudioUnitProperty_ElementCount - kAudioUnitScope_Input)");
    
}

void UnitMixer::setVolume(AudioUnitParameterValue volume, uint32_t bus) {
    
    XThrowIfError(AudioUnitSetParameter(_unit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, bus, volume, 0), "AudioUnitSetParameter(kMultiChannelMixerParam_Volume)");
    
}

bool UnitMixer::isEnabled(uint32_t bus) {
    
    AudioUnitParameterValue value;
    XThrowIfError(AudioUnitGetParameter(_unit, kMultiChannelMixerParam_Enable, kAudioUnitScope_Input, bus, &value), "AudioUnitSetParameter(kMultiChannelMixerParam_Enable)");
    return value;
    
}

void UnitMixer::setEnabled(bool enabled, uint32_t bus) {
    
    XThrowIfError(AudioUnitSetParameter(_unit, kMultiChannelMixerParam_Enable, kAudioUnitScope_Input, bus, (enabled ? 1.0 : 0.0), 0), "AudioUnitSetParameter(kMultiChannelMixerParam_Enable)");
    
}
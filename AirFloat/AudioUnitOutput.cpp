//
//  AudioUnitOutput.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#include "CAXException.h"
#include "AudioUnitOutput.h"

using namespace Audio;

UnitOutput::UnitOutput(Graph *inGraph) : Unit(kAudioUnitType_Output, kAudioUnitSubType_RemoteIO, kAudioUnitManufacturer_Apple, inGraph) {
    
}

bool UnitOutput::getOutputEnabled() {
    
    Boolean enabled;
    UInt32 size = sizeof(enabled);
    XThrowIfError(AudioUnitGetProperty(_unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enabled, &size), "AudioUnitGetProperty(kAudioOutputUnitProperty_EnableIO)");
    return size;
    
}

void UnitOutput::setOutputEnabled(bool enabled) {
    
    Boolean value = enabled;
    XThrowIfError(AudioUnitSetProperty(_unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &value, sizeof(value)), "AudioUnitSetProperty(kAudioOutputUnitProperty_EnabledUI)");
    
}
//
//  AudioUnitConverter.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#include "CAXException.h"
#include "AudioUnitConverter.h"

using namespace Audio;

UnitConverter::UnitConverter(AudioStreamBasicDescription inputDescription, AudioStreamBasicDescription outputDescription, Graph *inGraph) : Unit(kAudioUnitType_FormatConverter, kAudioUnitSubType_AUConverter, kAudioUnitManufacturer_Apple, inGraph) {
    
    XThrowIfError(AudioUnitSetProperty(_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &inputDescription, sizeof(inputDescription)), "AudioUnitSetProperty(kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input)");
    XThrowIfError(AudioUnitSetProperty(_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &outputDescription, sizeof(outputDescription)), "AudioUnitSetProperty(kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output)");
    
}

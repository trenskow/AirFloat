//
//  AudioUnitVariSpeed.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/19/13.
//
//

#include "CAXException.h"
#include "AudioUnitVariSpeed.h"

using namespace Audio;

VarispeedUnit::VarispeedUnit(Graph *inGraph) : Unit(kAudioUnitType_FormatConverter, kAudioUnitSubType_Varispeed, kAudioUnitManufacturer_Apple, inGraph) {
    
    _playbackRate = 1.0;
        
}

AudioUnitParameterValue VarispeedUnit::getPlaybackRate() {
    
    return _playbackRate;
    
}

void VarispeedUnit::setPlaybackRate(AudioUnitParameterValue playbackRate) {
    
    XThrowIfError(AudioUnitSetParameter(_unit, kVarispeedParam_PlaybackRate, kAudioUnitScope_Global, 0, playbackRate, 0), "AudioUnitSetParameter(kVarispeedParam_PlaybackRate)");
    _playbackRate = playbackRate;
    
}
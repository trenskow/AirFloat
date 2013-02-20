//
//  AudioUnitVarispeed.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/19/13.
//
//

#ifndef _AirFloat_AudioUnitVarispeed_h
#define _AirFloat_AudioUnitVarispeed_h

#include "AudioUnit.h"

namespace Audio {
    
    class Graph;
    
    class VarispeedUnit : public Unit {
        
    public:
        VarispeedUnit(Graph *inGraph);
        
        AudioUnitParameterValue getPlaybackRate();
        void setPlaybackRate(AudioUnitParameterValue playbackRate);
        
    private:
        AudioUnitParameterValue _playbackRate;
        
    };
    
}

#endif

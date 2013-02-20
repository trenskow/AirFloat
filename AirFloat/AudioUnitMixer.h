//
//  AudioUnitMixer.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#ifndef AirFloat_AudioUnitMixer_h
#define AirFloat_AudioUnitMixer_h

#include "AudioUnit.h"

namespace Audio {
    
    class Graph;
    
    class UnitMixer : public Unit {
        
    public:
        UnitMixer(uint32_t busCount, Graph* inGraph);
        
        void setVolume(AudioUnitParameterValue volume, uint32_t bus);
        bool isEnabled(uint32_t bus);
        void setEnabled(bool enabled, uint32_t bus);

    };
    
}
#endif

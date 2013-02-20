//
//  AudioUnitConverter.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#ifndef AirFloat_AudioUnitConverter_h
#define AirFloat_AudioUnitConverter_h

#include "AudioUnit.h"

namespace Audio {
    
    class Graph;
    
    class UnitConverter : public Unit {
        
    public:
        UnitConverter(AudioStreamBasicDescription inputDescription, AudioStreamBasicDescription outputDescription, Graph* inGraph);
        
        
    };
    
}

#endif

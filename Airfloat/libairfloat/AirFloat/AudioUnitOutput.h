//
//  AudioUnitOutput.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#ifndef AirFloat_AudioUnitOutput_h
#define AirFloat_AudioUnitOutput_h

#include "AudioUnit.h"

namespace Audio {
    
    class Graph;
    
    class UnitOutput : public Unit {
        
    public:
        UnitOutput(Graph *inGraph);
        
        bool getOutputEnabled();
        void setOutputEnabled(bool enabled);
    
    };
    
}

#endif

//
//  AudioUnit.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#ifndef AirFloat_AudioUnit_h
#define AirFloat_AudioUnit_h

#include <AudioToolbox/AudioToolbox.h>

namespace Audio {
    
    class Graph;
    class UnitConverter;
    
    class Unit {
        
    public:
        Unit(OSType type, OSType subtype, OSType manufacturer, Graph *inGraph);
        virtual ~Unit();
        
        uint32_t getInputBusCount();
        uint32_t getOutputBusCount();
        
        void connect(Unit *unit, uint32_t outputBus, uint32_t inputBus);
        void setInputCallback(uint32_t inputBus, AURenderCallback callback, void *ctx);
        
        AudioStreamBasicDescription getAudioDescriptionForInputBus(uint32_t inputBus);
        AudioStreamBasicDescription getAudioDescriptionForOutputBus(uint32_t outputBus);
        bool setAudioDescriptionForInputBus(AudioStreamBasicDescription description, uint32_t inputBus);
        
    protected:
        
        AudioUnit _unit;
        AUNode _node;
                
    private:
        
        Graph *_graph;
        
        uint32_t _busCountForScope(AudioUnitScope scope);
        AudioStreamBasicDescription _getAudioDescriptionForBus(AudioUnitScope scope, uint32_t bus);
        
        UnitConverter **_converterUnits;
        uint32_t _converterUnitesCount;
        
    };

}

#endif

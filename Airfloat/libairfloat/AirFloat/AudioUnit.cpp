//
//  AudioUnit.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#include "CAXException.h"

#include "AudioGraph.h"
#include "AudioUnitConverter.h"
#include "AudioUnit.h"

using namespace Audio;

Unit::Unit(OSType type, OSType subtype, OSType manufacturer, Graph *inGraph) {
    
    AudioComponentDescription description;
    bzero(&description, sizeof(description));
    description.componentType = type;
    description.componentSubType = subtype;
    description.componentManufacturer = manufacturer;
    
    XThrowIfError(AUGraphAddNode(inGraph->_graph, &description, &_node), "AUGraphAddNode");
    XThrowIfError(AUGraphNodeInfo(inGraph->_graph, _node, NULL, &_unit), "AUGraphNodeInfo");
    XThrowIfError(AudioUnitSetProperty(_unit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &inGraph->_maximumFramesPerSlice, sizeof(inGraph->_maximumFramesPerSlice)), "AudioUnitSetProperty(kAudioUnitProperty_MaximumFramesPerSlice)");
    _graph = inGraph;
    
    _converterUnits = NULL;
    _converterUnitesCount = 0;
    
}

Unit::~Unit() {
    
    for (uint32_t i = 0 ; i < _converterUnitesCount ; i++)
        delete _converterUnits[i];
    
}

uint32_t Unit::_busCountForScope(AudioUnitScope scope) {
    
    UInt32 busCount = 0;
    UInt32 size = sizeof(busCount);
    XThrowIfError(AudioUnitGetProperty(_unit, kAudioUnitProperty_ElementCount, scope, 0, &busCount, &size), "AudioUnitGetProperty(kAudioUnitProperty_ElementCount");
    return busCount;
    
}

AudioStreamBasicDescription Unit::_getAudioDescriptionForBus(AudioUnitScope scope, uint32_t bus) {
    
    AudioStreamBasicDescription description;
    UInt32 size = sizeof(description);
    bzero(&description, size);
    XThrowIfError(AudioUnitGetProperty(_unit, kAudioUnitProperty_StreamFormat, scope, bus, &description, &size), "AudioUnitGetProperty(kAudioUnitProperty_StreamFormat)");
    return description;
    
}

uint32_t Unit::getInputBusCount() {
    
    return this->_busCountForScope(kAudioUnitScope_Input);
    
}

uint32_t Unit::getOutputBusCount() {
    
    return this->_busCountForScope(kAudioUnitScope_Output);
    
}

void Unit::connect(Unit *unit, uint32_t outputBus, uint32_t inputBus) {
    
    AudioStreamBasicDescription outputDescription = unit->_getAudioDescriptionForBus(kAudioUnitScope_Output, outputBus);
    AudioStreamBasicDescription inputDescription = this->_getAudioDescriptionForBus(kAudioUnitScope_Input, inputBus);
    
    if (memcmp(&outputDescription, &inputDescription, sizeof(AudioStreamBasicDescription)) != 0) {
        
        OSStatus err = AudioUnitSetProperty(_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, inputBus, &outputDescription, sizeof(outputDescription));
        
        if (err != noErr) {
            if (!inputDescription.mSampleRate) {
                inputDescription.mSampleRate = outputDescription.mSampleRate;
                XThrowIfError(AudioUnitSetProperty(_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, inputBus, &inputDescription, sizeof(inputDescription)), "AudioUnitSetPropert(kAudioUnitProperty_StreamFormat (kAudioUnitScope_Input)");
            }
            
            UnitConverter *converter = new UnitConverter(outputDescription, inputDescription, _graph);
            converter->connect(unit, outputBus, 0);
            this->connect(converter, 0, inputBus);
            
            _converterUnits = (UnitConverter **)realloc(_converterUnits, sizeof(UnitConverter *) * (_converterUnitesCount + 1));
            _converterUnits[_converterUnitesCount] = converter;
            _converterUnitesCount++;
        } else
            XThrowIfError(AUGraphConnectNodeInput(_graph->_graph, unit->_node, outputBus, _node, inputBus), "AUGraphConnectNodeInput");
        
    } else
        XThrowIfError(AUGraphConnectNodeInput(_graph->_graph, unit->_node, outputBus, _node, inputBus), "AUGraphConnectNodeInput");
    
}

void Unit::setInputCallback(uint32_t inputBus, AURenderCallback callback, void *ctx) {
    
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = callback;
    callbackStruct.inputProcRefCon = ctx;
    XThrowIfError(AUGraphSetNodeInputCallback(_graph->_graph, this->_node, inputBus, &callbackStruct), "AUGraphSetNodeInputCallback");
    
}

AudioStreamBasicDescription Unit::getAudioDescriptionForInputBus(uint32_t inputBus) {
    
    return this->_getAudioDescriptionForBus(kAudioUnitScope_Input, inputBus);
    
}

AudioStreamBasicDescription Unit::getAudioDescriptionForOutputBus(uint32_t outputBus) {
    
    return this->_getAudioDescriptionForBus(kAudioUnitScope_Output, outputBus);
    
}

bool Unit::setAudioDescriptionForInputBus(AudioStreamBasicDescription description, uint32_t inputBus) {
    
    OSStatus err = AudioUnitSetProperty(_unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, inputBus, &description, sizeof(description));
    
    return (err == noErr);
    
}

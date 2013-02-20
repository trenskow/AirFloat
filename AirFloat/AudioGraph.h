//
//  AudioGraph.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#ifndef AirFloat_AudioGraph_h
#define AirFloat_AudioGraph_h

#include <AudioToolbox/AudioToolbox.h>

namespace Audio {
    
    class Unit;
    
    class Graph {
        
        friend class Unit;
        
    public:
        Graph(uint32_t maximumFramesPerSlice);
        ~Graph();
        
        void inititializeAndUpdate();
        
        bool isRunning();
        void setRunning(bool running);
        
    private:
        AUGraph _graph;
        uint32_t _maximumFramesPerSlice;
        
    };
    
}

#endif

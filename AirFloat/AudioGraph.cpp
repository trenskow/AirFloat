//
//  AudioGraph.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/13.
//
//

#include "CAXException.h"
#include "AudioGraph.h"

using namespace Audio;

Graph::Graph(uint32_t maximumFramesPerSlice) {
    
    XThrowIfError(NewAUGraph(&_graph), "NewAUGraph");
    XThrowIfError(AUGraphOpen(_graph), "AUGraphOpen");
    _maximumFramesPerSlice = maximumFramesPerSlice;
    
}

Graph::~Graph() {
    
    XThrowIfError(AUGraphUninitialize(_graph), "AUGraphUninitialize");
    XThrowIfError(DisposeAUGraph(_graph), "DisposeAUGraph");
    
}

void Graph::inititializeAndUpdate() {
    
    XThrowIfError(AUGraphInitialize(_graph), "AUGraphInitialize");
    XThrowIfError(AUGraphUpdate(_graph, NULL), "AUGraphUpdate");
    
}

bool Graph::isRunning() {
    
    Boolean isRunning;
    XThrowIfError(AUGraphIsRunning(_graph, &isRunning), "AUGraphIsRunning");
    return isRunning;
    
}

void Graph::setRunning(bool running) {
    
    if (running != this->isRunning()) {
        if (running)
            XThrowIfError(AUGraphStart(_graph), "AUGraphStart");
        else
            XThrowIfError(AUGraphStop(_graph), "AUGraphStop");
    }
    
}

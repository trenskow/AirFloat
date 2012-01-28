//
//  RAOPParameters.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __RAOPPARAMETER_H
#define __RAOPPARAMETER_H

typedef enum {
    
    ParameterTypeTextParameters = 0,
    ParameterTypeSDP,
    ParameterTypeDigest,
    ParameterTypeUnknown
    
} ParameterType;

class RAOPParameters {
    
public:
    RAOPParameters(const char* buffer, long length, ParameterType type, char delimiter = '\n');
    ~RAOPParameters();
    
    long count();
    const char* parameterAtIndex(long index);
    const char* valueForParameter(const char* name);
    void removeParameter(const char* name);
    
private:
    
    void _parseBufferParams(const char* bufer, long length);
    void _parseBufferSDP(const char* buffer, long length, char delimiter);
    void _parseBufferDigest(const char* buffer, long length);
    
    struct parameter {
        char* name;
        char* value;
    };
    
    parameter* _parameters;
    long _parameterCount;
    
};

#endif
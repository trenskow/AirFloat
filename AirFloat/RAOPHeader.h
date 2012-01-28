//
//  RAOPHeader.h
//  AirFloatCF
//
//  Created by Kristian Trenskow on 5/10/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#ifndef __RAOPHEADER_H
#define __RAOPHEADER_H

class RAOPHeader {
  
public:
    RAOPHeader(char* buffer, long length);
    ~RAOPHeader();
    
    long usedBuffer();
    const char* valueForName(const char* name);
    
private:

    void _parseBuffer(char* buffer, long length);
    
    struct header {
        char* name;
        char* value;
    };
    
    header* _headers;
    int _headerCount;
    long _usedBuffer;
    
};

#endif
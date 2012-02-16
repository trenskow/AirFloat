//
//  DMAPParser.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/12/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_DMAPParser_h
#define AirFloat_DMAPParser_h

#include <stdint.h>

typedef enum DMAPType {
    
    kDMAPTypeUnknown = 0,
    kDMAPTypeChar = 1,
    kDMAPTypeShort = 3,
    kDMAPTypeLong = 5,
    kDMAPTypeLongLong = 7,
    kDMAPTypeString = 9,
    kDMAPTypeDate = 10,
    kDMAPTypeVersion = 11,
    kDMAPTypeContainer = 12
    
} DMAPType;

typedef struct DMAPAtomType {
    
    const uint32_t tag;
    const char* const identifier;
    const DMAPType type;
    
} DMAPAtomType;

class DMAPParser {
    
public:
    static const DMAPAtomType const atomTypes[];
    static const uint32_t atomTypeCount;
    
    DMAPParser(const void* buffer, uint32_t size);
    ~DMAPParser();
    
    static DMAPType typeForTag(uint32_t tag);
    static uint32_t tagForTagIdentifier(const char* identifier);
    
    uint32_t count();
    uint32_t tagAtIndex(uint32_t index);
    
    DMAPParser* containerForAtom(uint32_t tag);
    DMAPParser* containerForIdentifier(const char* identifier);
    const char* stringForAtom(uint32_t tag);
    const char* stringForIdentifier(const char* identifier);
    char charForAtom(uint32_t tag);
    char charForIdentifier(const char* identifier);
    const void* bufferForAtom(uint32_t tag);
    const void* bufferForIdentifier(const char* identifier);
    
private:
    
    struct DMAPAtom {
        
        uint32_t tag;
        uint32_t type;
        void* buffer;
        uint32_t size;
        DMAPParser* container;
        
    } *_atoms;
    uint32_t _atomCount;
    
    void* _atomForTag(uint32_t tag);
    
};

#endif

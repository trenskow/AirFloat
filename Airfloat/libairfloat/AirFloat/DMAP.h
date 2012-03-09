//
//  DMAP.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/12/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef AirFloat_DMAP_h
#define AirFloat_DMAP_h

#include <stdint.h>
#include "Mutex.h"

#define DMAP_INDEX_NOT_FOUND UINT32_MAX

class DMAP;

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

typedef struct {
    
    const uint32_t tag;
    const char* const identifier;
    const DMAPType type;
    
} DMAPAtomType;

struct DMAPAtom {
    
    uint32_t tag;
    uint32_t type;
    void* buffer;
    uint32_t size;
    DMAP* container;
    
};

typedef struct {
    
    uint16_t major;
    uint8_t minor;
    uint8_t patch;
    
} DMAPVersion;

class DMAP {
    
public:
    static const DMAPAtomType const atomTypes[];
    static const uint32_t atomTypeCount;
    
    DMAP();
    DMAP(const void* buffer, uint32_t size);
    ~DMAP();
    
    DMAP* copy();
    
    static DMAPType typeForTag(uint32_t tag);
    static uint32_t tagForTagIdentifier(const char* identifier);
    static const char* tagIdentifierForTag(uint32_t tag);
    
    void parse(const void* buffer, uint32_t size);
    
    uint32_t getCount();
    uint32_t getTagAtIndex(uint32_t index);
    uint32_t getIndexOfTag(uint32_t tag);
    
    uint32_t getLength();
    uint32_t getContent(void* content, uint32_t size);
    
    uint32_t getSizeOfAtomAtIndex(uint32_t index);
    uint32_t getSizeOfAtom(uint32_t tag);
    uint32_t getSizeOfAtom(const char* identifier);
    
    char charAtIndex(uint32_t index);
    char charForAtom(uint32_t tag);
    char charForAtom(const char* identifier);
    int16_t shortAtIndex(uint32_t index);
    int16_t shortForAtom(uint32_t tag);
    int16_t shortForAtom(const char* identifier);
    int32_t longAtIndex(uint32_t index);
    int32_t longForAtom(uint32_t tag);
    int32_t longForAtom(const char* identifier);
    int64_t longlongAtIndex(uint32_t index);
    int64_t longlongForAtom(uint32_t tag);
    int64_t longlongForAtom(const char* identifier);
    const char* stringAtIndex(uint32_t index);
    const char* stringForAtom(uint32_t tag);
    const char* stringForAtom(const char* identifier);
    uint32_t dateAtIndex(uint32_t index);
    uint32_t dateForAtom(uint32_t tag);
    uint32_t dateForAtom(const char* identifier);
    DMAPVersion versionAtIndex(uint32_t index);
    DMAPVersion versionForAtom(uint32_t tag);
    DMAPVersion versionForAtom(const char* identifier);
    DMAP* containerAtIndex(uint32_t index);
    DMAP* containerForAtom(uint32_t tag);
    DMAP* containerForAtom(const char* identifier);
    const void* bytesAtIndex(uint32_t index);
    const void* bytesForAtom(uint32_t tag);
    const void* bytesForAtom(const char* identifier);
    
    void addChar(int8_t chr, uint32_t tag);
    void addShort(int16_t shrt, uint32_t tag);
    void addLong(int32_t lng, uint32_t tag);
    void addLonglong(int64_t lnglng, uint32_t tag);
    void addString(const char* string, uint32_t tag);
    void addDate(uint32_t date, uint32_t tag);
    void addVersion(DMAPVersion version, uint32_t tag);
    void addContainer(DMAP* container, uint32_t tag);
    void addBytes(const void* data, uint32_t size, uint32_t tag);
    
private:
        
    DMAPAtom *_atoms;
    uint32_t _atomCount;
    
    DMAPAtom* _atomForTag(uint32_t tag);
    DMAPAtom* _addAtom(uint32_t tag, uint32_t type = kDMAPTypeUnknown);
    
    void _setAtomBuffer(DMAPAtom* atom, const void* buffer, uint32_t size);
    
};

#endif

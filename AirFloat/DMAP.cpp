//
//  DMAP.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/12/12
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <assert.h>
#include <string.h>

extern "C" {
#include "Endian.h"
}

#include "Log.h"
#include "DMAP.h"

const DMAPAtomType DMAP::atomTypes[] = {
    { 'miid',     "dmap.itemid",                           kDMAPTypeLong      },
    { 'minm',     "dmap.itemname",                         kDMAPTypeString    },
    { 'mikd',     "dmap.itemkind",                         kDMAPTypeLongLong  },
    { 'mper',     "dmap.persistentid",                     kDMAPTypeUnknown   },
    { 'mcon',     "dmap.container",                        kDMAPTypeContainer },
    { 'mcti',     "dmap.containeritemid",                  kDMAPTypeLong      },
    { 'mpco',     "dmap.parentcontainerid",                kDMAPTypeLong      },
    { 'mstt',     "dmap.status",                           kDMAPTypeLong      },
    { 'msts',     "dmap.statusstring",                     kDMAPTypeString    },
    { 'mimc',     "dmap.itemcount",                        kDMAPTypeLong      },
    { 'mctc',     "dmap.containercount",                   kDMAPTypeLong      },
    { 'mrco',     "dmap.returnedcount",                    kDMAPTypeLong      },
    { 'mtco',     "dmap.specifiedtotalcount",              kDMAPTypeLong      },
    { 'f?ch',     "dmap.haschildcontainers",               kDMAPTypeChar      },
    { 'mlcl',     "dmap.listing",                          kDMAPTypeContainer },
    { 'mlit',     "dmap.listingitem",                      kDMAPTypeContainer },
    { 'mbcl',     "dmap.bag",                              kDMAPTypeContainer },
    { 'mdcl',     "dmap.dictionary",                       kDMAPTypeContainer },
    { 'msrv',     "dmap.serverinforesponse",               kDMAPTypeContainer },
    { 'msau',     "dmap.authenticationmethod",             kDMAPTypeChar      },
    { 'msas',     "dmap.authenticationschemes",            kDMAPTypeLong      },
    { 'mslr',     "dmap.loginrequired",                    kDMAPTypeChar      },
    { 'mpro',     "dmap.protocolversion",                  kDMAPTypeVersion   },
    { 'msal',     "dmap.supportsautologout",               kDMAPTypeChar      },
    { 'msup',     "dmap.supportsupdate",                   kDMAPTypeChar      },
    { 'mspi',     "dmap.supportspersistentids",            kDMAPTypeChar      },
    { 'msex',     "dmap.supportsextensions",               kDMAPTypeChar      },
    { 'msbr',     "dmap.supportsbrowse",                   kDMAPTypeChar      },
    { 'msqy',     "dmap.supportsquery",                    kDMAPTypeChar      },
    { 'msix',     "dmap.supportsindex",                    kDMAPTypeChar      },
    { 'msrs',     "dmap.supportsresolve",                  kDMAPTypeChar      },
    { 'mstm',     "dmap.timeoutinterval",                  kDMAPTypeLong      },
    { 'msdc',     "dmap.databasescount",                   kDMAPTypeLong      },
    { 'mstc',     "dmap.utctime",                          kDMAPTypeDate      },
    { 'mlog',     "dmap.loginresponse",                    kDMAPTypeContainer },
    { 'mlid',     "dmap.sessionid",                        kDMAPTypeLong      },
    { 'mupd',     "dmap.updateresponse",                   kDMAPTypeContainer },
    { 'musr',     "dmap.serverrevision",                   kDMAPTypeLong      },
    { 'muty',     "dmap.updatetype",                       kDMAPTypeChar      },
    { 'mudl',     "dmap.deletedid",                        kDMAPTypeContainer },
    { 'msdc',     "dmap.databasescount",                   kDMAPTypeLong      },
    { 'mccr',     "dmap.contentcodesresponse",             kDMAPTypeContainer },
    { 'mcnm',     "dmap.contentcodesnumber",               kDMAPTypeLong      },
    { 'mcna',     "dmap.contentcodesname",                 kDMAPTypeString    },
    { 'mcty',     "dmap.contentcodestype",                 kDMAPTypeShort     },
    { 'meds',     "dmap.editcommandssupported",            kDMAPTypeLong      },
    { 'ated',     "daap.supportsextradata",                kDMAPTypeShort     },
    { 'apro',     "daap.protocolversion",                  kDMAPTypeVersion   },
    { 'avdb',     "daap.serverdatabases",                  kDMAPTypeContainer },
    { 'abro',     "daap.databasebrowse",                   kDMAPTypeContainer },
    { 'adbs',     "daap.databasesongs",                    kDMAPTypeContainer },
    { 'aply',     "daap.databaseplaylists",                kDMAPTypeContainer },
    { 'apso',     "daap.playlistsongs",                    kDMAPTypeContainer },
    { 'arsv',     "daap.resolve",                          kDMAPTypeContainer },
    { 'arif',     "daap.resolveinfo",                      kDMAPTypeContainer },
    { 'abal',     "daap.browsealbumlisting",               kDMAPTypeContainer },
    { 'abar',     "daap.browseartistlisting",              kDMAPTypeContainer },
    { 'abcp',     "daap.browsecomposerlisting",            kDMAPTypeContainer },
    { 'abgn',     "daap.browsegenrelisting",               kDMAPTypeContainer },
    { 'aePP',     "com.apple.itunes.is-podcast-playlist",  kDMAPTypeChar      },
    { 'asal',     "daap.songalbum",                        kDMAPTypeString    },
    { 'asar',     "daap.songartist",                       kDMAPTypeString    },
    { 'asbr',     "daap.songbitrate",                      kDMAPTypeShort     },
    { 'ascm',     "daap.songcomment",                      kDMAPTypeString    },
    { 'asco',     "daap.songcompilation",                  kDMAPTypeChar      },
    { 'ascp',     "daap.songcomposer",                     kDMAPTypeString    },
    { 'asda',     "daap.songdateadded",                    kDMAPTypeDate      },
    { 'asdm',     "daap.songdatemodified",                 kDMAPTypeDate      },
    { 'asdc',     "daap.songdisccount",                    kDMAPTypeShort     },
    { 'asdn',     "daap.songdiscnumber",                   kDMAPTypeShort     },
    { 'aseq',     "daap.songeqpreset",                     kDMAPTypeString    },
    { 'asgn',     "daap.songgenre",                        kDMAPTypeString    },
    { 'asdt',     "daap.songdescription",                  kDMAPTypeString    },
    { 'assr',     "daap.songsamplerate",                   kDMAPTypeLong      },
    { 'assz',     "daap.songsize",                         kDMAPTypeLong      },
    { 'asst',     "daap.songstarttime",                    kDMAPTypeLong      },
    { 'assp',     "daap.songstoptime",                     kDMAPTypeLong      },
    { 'astm',     "daap.songtime",                         kDMAPTypeLong      },
    { 'astc',     "daap.songtrackcount",                   kDMAPTypeShort     },
    { 'astn',     "daap.songtracknumber",                  kDMAPTypeShort     },
    { 'asur',     "daap.songuserrating",                   kDMAPTypeChar      },
    { 'asyr',     "daap.songyear",                         kDMAPTypeShort     },
    { 'asfm',     "daap.songformat",                       kDMAPTypeString    },
    { 'asdb',     "daap.songdisabled",                     kDMAPTypeChar      },
    { 'asdk',     "daap.songdatakind",                     kDMAPTypeChar      },
    { 'asul',     "daap.songdataurl",                      kDMAPTypeString    },
    { 'asbt',     "daap.songbeatsperminute",               kDMAPTypeShort     },
    { 'abpl',     "daap.baseplaylist",                     kDMAPTypeChar      },
    { 'agrp',     "daap.songgrouping",                     kDMAPTypeString    },
    { 'ascd',     "daap.songcodectype",                    kDMAPTypeLong      },
    { 'ascs',     "daap.songcodecsubtype",                 kDMAPTypeLong      },
    { 'apsm',     "daap.playlistshufflemode",              kDMAPTypeChar      },
    { 'aprm',     "daap.playlistrepeatmode",               kDMAPTypeChar      },
    { 'asct',     "daap.songcategory",                     kDMAPTypeString    },
    { 'ascn',     "daap.songcontentdescription",           kDMAPTypeString    },
    { 'aslc',     "daap.songlongcontentdescription",       kDMAPTypeString    },
    { 'asky',     "daap.songkeywords",                     kDMAPTypeString    },
    { 'ascr',     "daap.songcontentrating",                kDMAPTypeChar      },
    { 'asgp',     "daap.songgapless",                      kDMAPTypeChar      },
    { 'asdr',     "daap.songdatereleased",                 kDMAPTypeDate      },
    { 'asdp',     "daap.songdatepurchased",                kDMAPTypeDate      },
    { 'ashp',     "daap.songhasbeenplayed",                kDMAPTypeChar      },
    { 'assn',     "daap.sortname",                         kDMAPTypeString    },
    { 'assa',     "daap.sortartist",                       kDMAPTypeString    },
    { 'assl',     "daap.sortalbumartist",                  kDMAPTypeString    },
    { 'assu',     "daap.sortalbum",                        kDMAPTypeString    },
    { 'assc',     "daap.sortcomposer",                     kDMAPTypeString    },
    { 'asss',     "daap.sortseriesname",                   kDMAPTypeString    },
    { 'asbk',     "daap.bookmarkable",                     kDMAPTypeChar      },
    { 'asbo',     "daap.songbookmark",                     kDMAPTypeLong      },
    { 'aspu',     "daap.songpodcasturl",                   kDMAPTypeString    },
    { 'asai',     "daap.songalbumid",                      kDMAPTypeLongLong  },
    { 'asls',     "daap.songlongsize",                     kDMAPTypeLongLong  },
    { 'asaa',     "daap.songalbumartist",                  kDMAPTypeString    },
    { 'aeNV',     "com.apple.itunes.norm-volume",          kDMAPTypeLong      },
    { 'aeSP',     "com.apple.itunes.smart-playlist",       kDMAPTypeChar      },
    { 'aeSI',     "com.apple.itunes.itms-songid",          kDMAPTypeLong      },
    { 'aeAI',     "com.apple.itunes.itms-artistid",        kDMAPTypeLong      },
    { 'aePI',     "com.apple.itunes.itms-playlistid",      kDMAPTypeLong      },
    { 'aeCI',     "com.apple.itunes.itms-composerid",      kDMAPTypeLong      },
    { 'aeGI',     "com.apple.itunes.itms-genreid",         kDMAPTypeLong      },
    { 'aeSF',     "com.apple.itunes.itms-storefrontid",    kDMAPTypeLong      },
    { 'aePC',     "com.apple.itunes.is-podcast",           kDMAPTypeChar      },
    { 'aeHV',     "com.apple.itunes.has-video",            kDMAPTypeChar      },
    { 'aeMK',     "com.apple.itunes.mediakind",            kDMAPTypeChar      },
    { 'aeSN',     "com.apple.itunes.series-name",          kDMAPTypeString    },
    { 'aeNN',     "com.apple.itunes.network-name",         kDMAPTypeString    },
    { 'aeEN',     "com.apple.itunes.episode-num-str",      kDMAPTypeString    },
    { 'aeES',     "com.apple.itunes.episode-sort",         kDMAPTypeLong      },
    { 'aeSU',     "com.apple.itunes.season-num",           kDMAPTypeLong      },
    { 'aeGH',     "com.apple.itunes.gapless-heur",         kDMAPTypeLong      },
    { 'aeGD',     "com.apple.itunes.gapless-enc-dr",       kDMAPTypeLong      },
    { 'aeGU',     "com.apple.itunes.gapless-dur",          kDMAPTypeLongLong  },
    { 'aeGR',     "com.apple.itunes.gapless-resy",         kDMAPTypeLongLong  },
    { 'aeGE',     "com.apple.itunes.gapless-enc-del",      kDMAPTypeLong      },
    { '????',     "com.apple.itunes.req-fplay",            kDMAPTypeChar      },
    { 'aePS',     "com.apple.itunes.special-playlist",     kDMAPTypeChar      },
    { 'aeCR',     "com.apple.itunes.content-rating",       kDMAPTypeString    },
    { 'aeSG',     "com.apple.itunes.saved-genius",         kDMAPTypeChar      },
    { 'aeHD',     "com.apple.itunes.is-hd-video",          kDMAPTypeChar      },
    { 'aeSE',     "com.apple.itunes.store-pers-id",        kDMAPTypeLongLong  },
    { 'aeDR',     "com.apple.itunes.drm-user-id",          kDMAPTypeLongLong  },
    { 'aeND',     "com.apple.itunes.non-drm-user-id",      kDMAPTypeLongLong  },
    { 'aeK1',     "com.apple.itunes.drm-key1-id",          kDMAPTypeLongLong  },
    { 'aeK2',     "com.apple.itunes.drm-key2-id",          kDMAPTypeLongLong  },
    { 'aeDV',     "com.apple.itunes.drm-versions",         kDMAPTypeLong      },
    { 'aeDP',     "com.apple.itunes.drm-platform-id",      kDMAPTypeLong      },
    { 'aeXD',     "com.apple.itunes.xid",                  kDMAPTypeString    },
    { 'aeMk',     "com.apple.itunes.extended-media-kind",  kDMAPTypeLong      },
    { 'aeAD',     "com.apple.itunes.adam-ids-array",       kDMAPTypeContainer },
    { 'aeSV',     "com.apple.itunes.music-sharing-version",kDMAPTypeLong      },
    
    // Custom
    
    { 'cmsr',     "com.AirFloat.RevisionNumber",           kDMAPTypeLong      },
    { 'cmst',     "com.AirFloat.NowPlayingContainer",      kDMAPTypeContainer },
    { 'caps',     "com.AirFloat.NowPlayingStatus",         kDMAPTypeChar      },
    { 'canp',     "com.AirFloat.NowPlayingInfo",           kDMAPTypeUnknown   },
    { 'cmpa',     "com.AirFloat.PairingContainer",         kDMAPTypeContainer },
    { 'cmnm',     "com.AirFloat.PairingDeviceName",        kDMAPTypeString    },
    { 'cmty',     "com.AirFloat.PairingDeviceType",        kDMAPTypeString    },
    { 'cmpg',     "com.AirFloat.PairingDeviceToken",       kDMAPTypeUnknown   }
    
};

const uint32_t DMAP::atomTypeCount = sizeof(DMAP::atomTypes) / sizeof(DMAPAtomType);

DMAP::DMAP() {
    
    _atoms = NULL;
    _atomCount = 0;
    
}

DMAP::DMAP(const void* buffer, uint32_t size) {
    
    _atoms = NULL;
    _atomCount = 0;
    
    parse(buffer, size);

}

DMAP::~DMAP() {
    
    for (uint32_t i = 0 ; i < _atomCount ; i++) {
        if (_atoms[i].type == kDMAPTypeContainer)
            delete _atoms[i].container;
        else
            free(_atoms[i].buffer);
    }
    
    free(_atoms);
    
}

DMAP* DMAP::copy() {
    
    DMAP* ret = new DMAP();
    
    ret->_atomCount = this->_atomCount;
    ret->_atoms = (DMAPAtom*)malloc(sizeof(DMAPAtom) * this->_atomCount);
    
    for (uint32_t i = 0 ; i < this->_atomCount ; i++) {
        
        ret->_atoms[i].tag = this->_atoms[i].tag;
        ret->_atoms[i].type = this->_atoms[i].type;
        ret->_atoms[i].buffer = NULL;
        ret->_atoms[i].size = this->_atoms[i].size;
        if (this->_atoms[i].buffer != NULL) {
            ret->_atoms[i].buffer = malloc(this->_atoms[i].size);
            memcpy(ret->_atoms[i].buffer, this->_atoms[i].buffer, this->_atoms[i].size);
        }
        if (this->_atoms[i].container != NULL)
            ret->_atoms[i].container = this->_atoms[i].container->copy();
        
    }
    
    return ret;
    
}

void DMAP::parse(const void* buffer, uint32_t size) {
    
    char* mBuffer = (char*)buffer;
    uint32_t length = size;
    
    while (length > 0) {
        
        uint32_t frameSize;
        uint32_t tag;
        
        memcpy(&frameSize, &mBuffer[4], sizeof(uint32_t));
        memcpy(&tag, mBuffer, sizeof(uint32_t));
        
        frameSize = btml(frameSize);
        tag = btml(tag);

        mBuffer += 8;
        length -= 8;
        
        _atoms = (DMAPAtom*)realloc(_atoms, sizeof(DMAPAtom) * (_atomCount + 1));
        
        _atoms[_atomCount].tag = tag;
        _atoms[_atomCount].type = typeForTag(tag);
        _atoms[_atomCount].buffer = malloc(frameSize + 1);
        ((char*)_atoms[_atomCount].buffer)[frameSize] = '\0';
        memcpy(_atoms[_atomCount].buffer, mBuffer, frameSize);
        _atoms[_atomCount].size = frameSize;
        
        _atoms[_atomCount].container = NULL;
        if (_atoms[_atomCount].type == kDMAPTypeContainer)
            _atoms[_atomCount].container = new DMAP(mBuffer, frameSize);
        
        _atomCount++;
        
        mBuffer += frameSize;
        length -= frameSize;
        
    }
    
}

uint32_t DMAP::getCount() {
    
    return _atomCount;
    
}

uint32_t DMAP::getTagAtIndex(uint32_t index) {
    
    assert(index < _atomCount);
    
    return _atoms[index].tag;
    
}

uint32_t DMAP::getIndexOfTag(uint32_t tag) {
    
    for (uint32_t i = 0 ; i < _atomCount ; i++)
        if (_atoms[i].tag == tag)
            return i;
    
    return DMAP_INDEX_NOT_FOUND;
    
}

uint32_t DMAP::getLength() {
    
    uint32_t ret = 0;
    
    for (uint32_t i = 0 ; i < _atomCount ; i++) {
        if (_atoms[i].type == kDMAPTypeContainer)
            ret += _atoms[i].container->getLength() + 8;
        else
            ret += _atoms[i].size + 8;
    }
    
    return ret;
    
}

uint32_t DMAP::getContent(void* content, uint32_t size) {
    
    uint32_t writePos = 0;
    
    for (uint32_t i = 0 ; i < _atomCount ; i++) {
        
        uint32_t atomSize = (_atoms[i].type == kDMAPTypeContainer ? _atoms[i].container->getLength() : _atoms[i].size);
        if (size - writePos >= atomSize + 8) {
            
            uint32_t tag = mtbl(_atoms[i].tag);
            uint32_t aSize = mtbl(atomSize);
            memcpy(&((char*)content)[writePos], &tag, 4);
            memcpy(&((char*)content)[writePos + 4], &aSize, 4);
            
            writePos += 8;
            
            if (_atoms[i].size > 0 || _atoms[i].type == kDMAPTypeContainer) {
                
                if (_atoms[i].type == kDMAPTypeContainer)
                    writePos += _atoms[i].container->getContent(&((char*)content)[writePos], size - writePos);
                else {
                    memcpy(&((char*)content)[writePos], _atoms[i].buffer, _atoms[i].size);
                    writePos += _atoms[i].size;
                }

            }
            
        }
        
    }
    
    return writePos;
    
}

DMAPType DMAP::typeForTag(uint32_t tag) {
    
    for (uint32_t i = 0 ; i < atomTypeCount ; i++)
        if (atomTypes[i].tag == tag)
            return atomTypes[i].type;
    
    return kDMAPTypeUnknown;
    
}

uint32_t DMAP::tagForTagIdentifier(const char* identifier) {
    
    for (uint32_t i = 0 ; i < atomTypeCount ; i++)
        if (strcmp(atomTypes[i].identifier, identifier) == 0)
            return atomTypes[i].tag;
    
    if (strlen(identifier) >= 4) {
        uint32_t ret = 0;
        memcpy(&ret, identifier, sizeof(uint32_t));
        return ret;
    }
    
    return 0;
    
}

const char* DMAP::tagIdentifierForTag(uint32_t tag) {
    
    for (uint32_t i = 0 ; i < atomTypeCount ; i++)
        if (atomTypes[i].tag == tag)
            return atomTypes[i].identifier;
    
    static char ret[sizeof(uint32_t) + 1];
    ret[sizeof(uint32_t)] = '\0';
    
    memcpy(ret, (char*)&tag, sizeof(uint32_t));
    
    return ret;
    
}

uint32_t DMAP::getSizeOfAtomAtIndex(uint32_t index) {
    
    assert(index < _atomCount);
    
    return _atoms[index].size;
    
}

uint32_t DMAP::getSizeOfAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return getSizeOfAtomAtIndex(index);
    
    return 0;
    
}

uint32_t DMAP::getSizeOfAtom(const char* identifier) {
    
    return getSizeOfAtom(tagForTagIdentifier(identifier));
    
}

char DMAP::charAtIndex(uint32_t index) {
    
    assert(index < _atomCount && _atoms[index].type == kDMAPTypeChar);
    
    return *((char*)_atoms[index].buffer);
    
}

char DMAP::charForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return charAtIndex(index);
    
    return 0;
    
}

char DMAP::charForAtom(const char* identifier) {
    
    return charForAtom(tagForTagIdentifier(identifier));
    
}

int16_t DMAP::shortAtIndex(uint32_t index) {
    
    assert(index < _atomCount && _atoms[index].type == kDMAPTypeShort);
    
    return btms(*((int16_t*)_atoms[index].buffer));
    
}

int16_t DMAP::shortForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return shortAtIndex(index);
    
    return 0;
    
}

int16_t DMAP::shortForAtom(const char* identifier) {
    
    return shortForAtom(tagForTagIdentifier(identifier));
    
}

int32_t DMAP::longAtIndex(uint32_t index) {
    
    assert(index < _atomCount && (_atoms[index].type == kDMAPTypeLong || _atoms[index].type == kDMAPTypeDate));
    
    return btml(*((int32_t*)_atoms[index].buffer));
    
}

int32_t DMAP::longForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return longAtIndex(index);
    
    return 0;
    
}

int32_t DMAP::longForAtom(const char* identifier) {
    
    return longForAtom(tagForTagIdentifier(identifier));
    
}

int64_t DMAP::longlongAtIndex(uint32_t index) {
    
    assert(index < _atomCount && _atoms[index].type == kDMAPTypeLongLong);
    
    return btmll(*((int64_t*)_atoms[index].buffer));
    
}

int64_t DMAP::longlongForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return longlongAtIndex(index);
    
    return 0;
    
}

int64_t DMAP::longlongForAtom(const char* identifier) {
    
    return longlongForAtom(tagForTagIdentifier(identifier));
    
}

const char* DMAP::stringAtIndex(uint32_t index) {
    
    assert(index < _atomCount && _atoms[index].type == kDMAPTypeString);
    
    return (char*) _atoms[index].buffer;
    
}

const char* DMAP::stringForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return stringAtIndex(index);
    
    return NULL;
    
}

const char* DMAP::stringForAtom(const char* identifier) {
    
    return stringForAtom(tagForTagIdentifier(identifier));
    
}

uint32_t DMAP::dateAtIndex(uint32_t index) {
    
    return longAtIndex(index);
    
}

uint32_t DMAP::dateForAtom(uint32_t tag) {
    
    return longForAtom(tag);
    
}

uint32_t DMAP::dateForAtom(const char* identifier) {
    
    return longForAtom(identifier);
    
}

DMAPVersion DMAP::versionAtIndex(uint32_t index) {
    
    assert(index < _atomCount && _atoms[index].type == kDMAPTypeVersion);
    
    DMAPVersion version = *((DMAPVersion*)_atoms[index].buffer);
    
    version.major = btms(version.major);
    
    return version;
    
}

DMAPVersion DMAP::versionForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return versionAtIndex(index);
    
    return (DMAPVersion){ 0, 0, 0 };
    
}

DMAPVersion DMAP::versionForAtom(const char* identifier) {
    
    return versionForAtom(tagForTagIdentifier(identifier));
    
}

DMAP* DMAP::containerAtIndex(uint32_t index) {
    
    assert(index < _atomCount && _atoms[index].type == kDMAPTypeContainer);
    
    return _atoms[index].container;
    
}

DMAP* DMAP::containerForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return _atoms[index].container;
    
    return NULL;
    
}

DMAP* DMAP::containerForAtom(const char* identifier) {
    
    return containerForAtom(tagForTagIdentifier(identifier));
    
}

const void* DMAP::bytesAtIndex(uint32_t index) {
    
    assert(index < _atomCount);
    
    return _atoms[index].buffer;
    
}

const void* DMAP::bytesForAtom(uint32_t tag) {
    
    uint32_t index = getIndexOfTag(tag);
    if (index != DMAP_INDEX_NOT_FOUND)
        return bytesAtIndex(index);
    
    return NULL;
    
}

const void* DMAP::bytesForAtom(const char* identifier) {
    
    return bytesForAtom(tagForTagIdentifier(identifier));
    
}

void DMAP::addChar(int8_t chr, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeChar);
    _setAtomBuffer(newAtom, &chr, sizeof(int8_t));
    
}

void DMAP::addShort(int16_t shrt, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeShort);
    int16_t val = mtbs(shrt);
    _setAtomBuffer(newAtom, &val, sizeof(int16_t));
    
}

void DMAP::addLong(int32_t lng, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeLong);
    int32_t val = mtbl(lng);
    _setAtomBuffer(newAtom, &val, sizeof(int32_t));
    
}

void DMAP::addLonglong(int64_t lnglng, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeLongLong);
    int64_t val = mtbll(lnglng);
    _setAtomBuffer(newAtom, &val, sizeof(int64_t));
    
}

void DMAP::addString(const char* string, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeString);
    _setAtomBuffer(newAtom, string, strlen(string));
    
}

void DMAP::addDate(uint32_t date, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeDate);
    uint32_t val = mtbl(date);
    _setAtomBuffer(newAtom, &val, sizeof(uint32_t));
    
}

void DMAP::addVersion(DMAPVersion version, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeVersion);
    
    DMAPVersion val = version;
    val.major = mtbs(val.major);
    
    _setAtomBuffer(newAtom, &val, sizeof(uint32_t));
    
}

void DMAP::addContainer(DMAP* container, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeContainer);
    newAtom->container = container->copy();
    
}

void DMAP::addBytes(const void* data, uint32_t size, uint32_t tag) {
    
    DMAPAtom* newAtom = _addAtom(tag, kDMAPTypeUnknown);
    _setAtomBuffer(newAtom, data, size);
    
}

DMAPAtom* DMAP::_atomForTag(uint32_t tag) {
    
    for (uint32_t i = 0 ; i < _atomCount ; i++)
        if (_atoms[i].tag == tag)
            return &_atoms[i];
    
    return NULL;
    
}

DMAPAtom* DMAP::_addAtom(uint32_t tag, uint32_t type) {
    
    _atoms = (DMAPAtom*)realloc(_atoms, sizeof(DMAPAtom) * (_atomCount + 1));
    DMAPAtom* ret = &_atoms[_atomCount];
    _atomCount++;
    bzero(ret, sizeof(DMAPAtom));
    ret->tag = tag;
    ret->type = type;
    
    return ret;
    
}

void DMAP::_setAtomBuffer(DMAPAtom* atom, const void* buffer, uint32_t size) {
    
    if (atom->buffer != NULL)
        free(atom->buffer);
    
    atom->size = size;
    atom->buffer = malloc(size + 1);
    bzero(atom->buffer, size + 1);
    memcpy(atom->buffer, buffer, size);
    
}

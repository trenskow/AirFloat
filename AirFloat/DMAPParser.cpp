//
//  DMAPParser.cpp
//  AirFloat
//
//  Created by Kristian Trenskow on 2/kDMAPTypeContainer/kDMAPTypeContainer.
//  Copyright (c) 20kDMAPTypeContainer The Famous Software Company. All rights reserved.
//

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "Log.h"
#include "DMAPParser.h"

const DMAPAtomType const DMAPParser::atomTypes[] = {
    { 'miid',     "dmap.itemid",                           kDMAPTypeLong      },
    { 'minm',     "dmap.itemname",                         kDMAPTypeString    },
    { 'mikd',     "dmap.itemkind",                         kDMAPTypeLongLong  },
    { 'mper',     "dmap.persistentid",                     kDMAPTypeString    },
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
    
    { 'cmst',     "com.AirFloat.NowPlayingContainer",      kDMAPTypeContainer },
    { 'caps',     "com.AirFloat.NowPlayingStatus",         kDMAPTypeChar      },
    { 'canp',     "com.AirFloat.NowPlayingInfo",           kDMAPTypeChar      }
    
};

const uint32_t DMAPParser::atomTypeCount = sizeof(DMAPParser::atomTypes) / sizeof(DMAPAtomType);

DMAPParser::DMAPParser(const void* buffer, uint32_t size) {
    
    log_data(LOG_INFO, (char*)buffer, size);
    
    _atoms = NULL;
    _atomCount = 0;
    
    char* mBuffer = (char*)buffer;
    uint32_t length = size;
    
    while (length > 0) {
        
        char* name = mBuffer;
        int frameSize = ntohl(*((uint32_t*)&mBuffer[4]));
        uint32_t tag = ntohl(*((uint32_t*)mBuffer));
        mBuffer += 8;
        length -= 8;
        
        _atoms = (DMAPAtom*)realloc(_atoms, sizeof(DMAPAtom) * (_atomCount + 1));
        memcpy(&_atoms[_atomCount].tag, name, 4);
        
        _atoms[_atomCount].tag = tag;
        _atoms[_atomCount].type = DMAPParser::typeForTag(tag);
        _atoms[_atomCount].buffer = malloc(frameSize + 1);
        ((char*)_atoms[_atomCount].buffer)[frameSize] = '\0';
        memcpy(_atoms[_atomCount].buffer, mBuffer, frameSize);
        _atoms[_atomCount].size = frameSize;
        
        _atoms[_atomCount].container = NULL;
        if (_atoms[_atomCount].type == kDMAPTypeContainer)
            _atoms[_atomCount].container = new DMAPParser(mBuffer, frameSize);
        
        _atomCount++;
        
        mBuffer += frameSize;
        length -= frameSize;
        
    }        

}

DMAPParser::~DMAPParser() {
    
    for (uint32_t i = 0 ; i < _atomCount ; i++) {
        free(_atoms[i].buffer);
        if (_atoms[i].container != NULL)
            delete _atoms[i].container;
    }
    
    free(_atoms);
    
}

uint32_t DMAPParser::count() {
    
    return _atomCount;
    
}

uint32_t DMAPParser::tagAtIndex(uint32_t index) {
    
    assert(index >= 0 && index < _atomCount);
    
    return _atoms[index].tag;
    
}

DMAPType DMAPParser::typeForTag(uint32_t tag) {
    
    for (uint32_t i = 0 ; i < DMAPParser::atomTypeCount ; i++)
        if (DMAPParser::atomTypes[i].tag == tag)
            return DMAPParser::atomTypes[i].type;
    
    return kDMAPTypeUnknown;
    
}

uint32_t DMAPParser::tagForTagIdentifier(const char* identifier) {
    
    for (uint32_t i = 0 ; i < DMAPParser::atomTypeCount ; i++)
        if (strcmp(DMAPParser::atomTypes[i].identifier, identifier) == 0)
            return DMAPParser::atomTypes[i].tag;
    
    return 0;
    
}

DMAPParser* DMAPParser::containerForAtom(uint32_t tag) {
    
    DMAPAtom* atom = (DMAPAtom*)_atomForTag(tag);
    if (atom != NULL) {
        assert(atom->type == kDMAPTypeContainer);
        return (DMAPParser*)atom->container;
    }
    
    return NULL;
    
}

DMAPParser* DMAPParser::containerForIdentifier(const char* identifier) {
    
    return containerForAtom(DMAPParser::tagForTagIdentifier(identifier));
    
}

const char* DMAPParser::stringForAtom(uint32_t tag) {
    
    DMAPAtom* atom = (DMAPAtom*)_atomForTag(tag);
    if (atom != NULL) {
        assert(atom->type == kDMAPTypeString);
        return (char*) atom->buffer;
    }
    
    return NULL;
    
}

const char* DMAPParser::stringForIdentifier(const char* identifier) {
    
    return stringForAtom(DMAPParser::tagForTagIdentifier(identifier));
    
}

char DMAPParser::charForAtom(uint32_t tag) {
    
    DMAPAtom* atom = (DMAPAtom*)_atomForTag(tag);
    if (atom != NULL) {
        assert(atom->type == kDMAPTypeChar);
        return *((char*)atom->buffer);
    }
    
    return 0;
    
}

char DMAPParser::charForIdentifier(const char* identifier) {
    
    return charForAtom(DMAPParser::tagForTagIdentifier(identifier));
    
}

const void* DMAPParser::bufferForAtom(uint32_t tag) {
    
    DMAPAtom* atom = (DMAPAtom*)_atomForTag(tag);
    if (atom != NULL)
        return atom->buffer;
    
    return NULL;
    
}

const void* DMAPParser::bufferForIdentifier(const char* identifier) {
    
    return bufferForAtom(DMAPParser::tagForTagIdentifier(identifier));
    
}

void* DMAPParser::_atomForTag(uint32_t tag) {
    
    for (uint32_t i = 0 ; i < _atomCount ; i++)
        if (_atoms[i].tag == tag)
            return &_atoms[i];
    
    return NULL;
    
}















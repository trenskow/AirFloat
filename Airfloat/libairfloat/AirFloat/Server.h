//
//  Library.h
//  AirFloatLibrary
//
//  Created by Kristian Trenskow on 4/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef __LIBRARY_H
#define __LIBRARY_H

extern "C" {
#include "settings.h"
#include "dmap.h"
#include "log.h"
#include "mutex.h"
#include "condition.h"
#include "notificationcenter.h"
}

#include "AudioConverter.h"
#include "AppleLosslessAudioConverter.h"
#include "AppleLosslessSoftwareAudioConverter.h"
#include "AudioPlayer.h"
#include "AudioPacket.h"
#include "AudioQueue.h"

extern "C" {
#include "base64.h"
#include "socket.h"
#include "sockaddr.h"
#include "webheaders.h"
#include "webresponse.h"
#include "webrequest.h"
#include "webconnection.h"
#include "webserver.h"
}

#include "RTPReceiver.h"
#include "RAOPParameters.h"
#include "RAOPConnection.h"
#include "RAOPServer.h"

#endif

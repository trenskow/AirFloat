//
//  AirFloatNoticationsHub.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/9/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "DMAPParser.h"
#import "RAOPServer.h"
#import "RAOPConnection.h"
#import "AudioQueue.h"

#import "NotificationCenter.h"
#import "AirFloatNotificationsHub.h"

static struct AirFloatNotificationBridge {
    const char* raopName;
    NSString* foundationName;
} _airFloatNotificationBridge[5] = {
    
    {
        RAOPServer::clientConnectedNotificationName,
        AirFloatClientConnectedNotification
    },
    {
        RAOPConnection::clientDisconnectedNotificationName,
        AirFloatClientDisconnectedNotification
    },
    {
        AudioQueue::flushNotificationName,
        AirFloatRecordingEndedNotification
    },
    {
        AudioQueue::syncNotificationName,
        AirFloatRecordingStartedNotification
    },
    {
        RAOPServer::localhostConnectedErrorNotificationName,
        AirFloatLocalhostConnectedErrorNotification
    }
    
};
static uint32_t _airFloatNotificationBridgeCount = sizeof(_airFloatNotificationBridge) / sizeof(AirFloatNotificationBridge);

@interface AirFloatNotificationsHub (Private)

- (void)_notificationReceived:(void*)sender name:(const char*)name notificationInfo:(void*)notificationInfo;

@end

static void notificationCallback(void* sender, const char* name, void* notificationInfo, void* callbackContext) {
    
    dispatch_block_t routeBlock = ^{
        [(AirFloatNotificationsHub*)callbackContext _notificationReceived:sender name:name notificationInfo:notificationInfo];
    };
    
    if (notificationInfo == NULL)
        dispatch_async(dispatch_get_main_queue(), routeBlock);
    else
        dispatch_sync(dispatch_get_main_queue(), routeBlock);
    
}

@implementation AirFloatNotificationsHub

- (id)init {
    
    if ((self = [super init]))
        NotificationCenter::defaultCenter()->addObserver(notificationCallback, self, NULL, NULL);
    
    return self;
    
}

- (void)dealloc {
    
    NotificationCenter::defaultCenter()->removeObserver(self);
    
    [super dealloc];
    
}

#pragma mark Notification handling

- (void)_notificationReceived:(void *)sender name:(const char *)name notificationInfo:(void *)notificationInfo {
    
    NSMutableDictionary* infoDictionary = [NSMutableDictionary dictionaryWithObject:[NSValue valueWithPointer:sender] forKey:kAirFloatSenderOriginKey];
    
    for (uint32_t i = 0 ; i < _airFloatNotificationBridgeCount ; i++)
        if (strcmp(name, _airFloatNotificationBridge[i].raopName) == 0) {
            [[NSNotificationCenter defaultCenter] postNotificationName:_airFloatNotificationBridge[i].foundationName object:self];
            return;
        }
    
    if (strcmp(name, RAOPConnection::clientUpdatedTrackInfoNofificationName) == 0) {

        DMAPParser* tags = (DMAPParser*)notificationInfo;
        
        [infoDictionary addEntriesFromDictionary:[NSDictionary dictionaryWithObjectsAndKeys:
                                                  [NSString stringWithCString:tags->stringForIdentifier("dmap.itemname") encoding:NSUTF8StringEncoding], kAirFloatTrackInfoTrackTitleKey, 
                                                  [NSString stringWithCString:tags->stringForIdentifier("daap.songartist") encoding:NSUTF8StringEncoding], kAirFloatTrackInfoArtistNameKey, 
                                                  [NSString stringWithCString:tags->stringForIdentifier("daap.songalbum") encoding:NSUTF8StringEncoding], kAirFloatTrackInfoAlbumNameKey, nil]];
        
        [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatTrackInfoUpdatedNotification object:self userInfo:infoDictionary];

        return;
        
    } else if (strcmp(name, RAOPConnection::clientUpdatedMetadataNotificationName) == 0) {
        
        RAOPConnectionClientUpdatedMetadataNotificationInfo* info = (RAOPConnectionClientUpdatedMetadataNotificationInfo*)notificationInfo;
        
        NSData* data = [NSData dataWithBytes:info->content length:info->contentLength];
        NSString* contentType = [NSString stringWithCString:info->contentType encoding:NSUTF8StringEncoding];
        
        [infoDictionary addEntriesFromDictionary:[NSDictionary dictionaryWithObjectsAndKeys:data, kAirFloatMetadataDataKey, contentType, kAirFloatMetadataContentType, nil]];
        
        [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatMetadataUpdatedNotification object:self userInfo:infoDictionary];
        
    }
    
}

@end

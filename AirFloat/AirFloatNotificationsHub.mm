//
//  AirFloatNoticationsHub.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/9/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "Server.h"

#import "AirFloatAdditions.h"
#import "AirFloatNotificationsHub.h"

static struct AirFloatNotificationBridge {
    NSString* raopName;
    NSString* foundationName;
} _airFloatNotificationBridge[5] = {
    
    {
        [[NSString alloc] initWithCString:RAOPConnection::clientConnectedNotificationName encoding:NSASCIIStringEncoding],
        AirFloatClientConnectedNotification
    },
    {
        [[NSString alloc] initWithCString:RAOPConnection::clientDisconnectedNotificationName encoding:NSASCIIStringEncoding],
        AirFloatClientDisconnectedNotification
    },
    {
        [[NSString alloc] initWithCString:AudioQueue::flushNotificationName encoding:NSASCIIStringEncoding],
        AirFloatRecordingEndedNotification
    },
    {
        [[NSString alloc] initWithCString:AudioQueue::syncNotificationName encoding:NSASCIIStringEncoding],
        AirFloatRecordingStartedNotification
    },
    {
        [[NSString alloc] initWithCString:RAOPServer::localhostConnectedErrorNotificationName encoding:NSASCIIStringEncoding],
        AirFloatLocalhostConnectedErrorNotification
    }
    
};
static uint32_t _airFloatNotificationBridgeCount = sizeof(_airFloatNotificationBridge) / sizeof(AirFloatNotificationBridge);

@interface AirFloatNotificationsHub (Private)

- (void)_notificationReceived:(void*)sender name:(NSString*)name notificationInfo:(void*)notificationInfo;

@end

static void notificationCallback(notification_p notification, void* ctx) {
    
    dispatch_block_t routeBlock = ^{
        [(AirFloatNotificationsHub*)ctx _notificationReceived:notification_get_sender(notification)
                                                         name:[NSString stringWithCString:notification_get_name(notification) encoding:NSASCIIStringEncoding]
                                             notificationInfo:notification_get_info(notification)];
    };
    
    if (notification_get_info(notification) == NULL)
        dispatch_async(dispatch_get_main_queue(), routeBlock);
    else
        dispatch_sync(dispatch_get_main_queue(), routeBlock);
    
}

@implementation AirFloatNotificationsHub

- (id)init {
    
    if ((self = [super init]))
        notification_center_add_observer(notificationCallback, self, NULL, NULL);
    
    return self;
    
}

- (void)dealloc {
    
    notification_center_remove_observer(NULL, self);
    
    [super dealloc];
    
}

#pragma mark - Notification handling

- (void)_notificationReceived:(void *)sender name:(NSString*)name notificationInfo:(void *)notificationInfo {
    
    NSMutableDictionary* infoDictionary = [NSMutableDictionary dictionaryWithObject:[NSValue valueWithPointer:sender] forKey:kAirFloatSenderOriginKey];
    
    for (uint32_t i = 0 ; i < _airFloatNotificationBridgeCount ; i++)
        if ([name isEqualToString:_airFloatNotificationBridge[i].raopName]) {
            [NSDefaultNotificationCenter postNotificationName:_airFloatNotificationBridge[i].foundationName object:self userInfo:infoDictionary];
            return;
        }
    
    if ([name isEqualToString:[NSString stringWithCString:RAOPConnection::clientUpdatedTrackInfoNofificationName encoding:NSASCIIStringEncoding]]) {

        dmap_p tags = (dmap_p)notificationInfo;
        
        [infoDictionary addEntriesFromDictionary:[NSDictionary dictionaryWithObjectsAndKeys:
                                                  [NSString stringWithCString:dmap_string_for_atom_identifer(tags, "dmap.itemname") encoding:NSUTF8StringEncoding], kAirFloatTrackInfoTrackTitleKey,
                                                  [NSString stringWithCString:dmap_string_for_atom_identifer(tags, "daap.songartist") encoding:NSUTF8StringEncoding], kAirFloatTrackInfoArtistNameKey,
                                                  [NSString stringWithCString:dmap_string_for_atom_identifer(tags, "daap.songalbum") encoding:NSUTF8StringEncoding], kAirFloatTrackInfoAlbumNameKey, nil]];
        
        [NSDefaultNotificationCenter postNotificationName:AirFloatTrackInfoUpdatedNotification object:self userInfo:infoDictionary];

        return;
        
    } else if ([name isEqualToString:[NSString stringWithCString:RAOPConnection::clientUpdatedMetadataNotificationName encoding:NSASCIIStringEncoding]]) {
        
        RAOPConnectionClientUpdatedMetadataNotificationInfo* info = (RAOPConnectionClientUpdatedMetadataNotificationInfo*)notificationInfo;
        
        NSData* data = [NSData dataWithBytes:info->content length:info->contentLength];
        NSString* contentType = [NSString stringWithCString:info->contentType encoding:NSUTF8StringEncoding];
        
        [infoDictionary addEntriesFromDictionary:[NSDictionary dictionaryWithObjectsAndKeys:data, kAirFloatMetadataDataKey, contentType, kAirFloatMetadataContentType, nil]];
        
        [NSDefaultNotificationCenter postNotificationName:AirFloatMetadataUpdatedNotification object:self userInfo:infoDictionary];
        
    }
    
}

@end

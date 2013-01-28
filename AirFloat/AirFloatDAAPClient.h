//
//  AirFloatDAAPClient.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum {
    
    kAirFloatDAAPPlaybackStatusUnknown = 0,
    kAirFloatDAAPPlaybackStatusPaused = 2,
    kAirFloatDAAPPlaybackStatusPlaying = 4
    
} AirFloatDAAPPlaybackStatus;

typedef enum {
    
    kAirFloatDAAPClientTypeDACP = 0,
    kAirFloatDAAPClientTypeDAAP,
    
} AirFloatDAAPClientType;

#define AirFloatDAAPClientIsControllableNotification @"AirFloatDAAPClientIsControllableNotification"
#define AirFloatDAAPPlaybackStatusUpdatedNotification @"AirFloatDAAPPlaybackStatusUpdatedNotification"

#define kAirFloatDAAPPlaybackStatusKey @"kAirFloatDAAPPlaybackStatusKey"

#define AirFloatDAAPClientFailedAuthenticationNotification @"AirFloatDAAPClientFailedAuthenticationNotification"

#define kAirFloatDAAPClientGuidKey @"kAirFloatDAAPClientGuidKey"

#define AirFloatDAAPPlaybackPlayPauseNotification @"AirFloatPlaybackPlayPauseNotification"
#define AirFloatDAAPPlaybackNextNotification @"AirFloatPlaybackNextNotification"
#define AirFloatDAAPPlaybackPrevNotification @"AirFloatPlaybackPrevNotification"

#define kAirFloatDAAPPlaylistDatabaseIdentifierKey @"kAirFloatDAAPPlaylistDatabaseIdentifierKey"
#define kAirFloatDAAPPlaylistIdentifierKey @"kAirFloatDAAPPlaylistIdentifierKey"
#define kAirFloatDAAPPlaylistItemsKey @"kAirFloatDAAPPlaylistItemsKey"
#define kAirFloatDAAPPlaylistItemsPlayingItemKey @"kAirFloatDAAPPlaylistItemsPlayingSongIndexKey"
#define kAirFloatDAAPPlaylistItemsPreviouslyPlayedItemKey @"kAirFloatDAAPPlaylistItemsPreviouslyPlayedSongIndexKey"

#define kAirFloatDAAPItemIdenfifierKey @"kAirFloatDAAPItemIdenfifierKey"
#define kAirFloatDAAPItemDurationKey @"kAirFloatDAAPItemDurationKey"
#define kAirFloatDAAPItemNameKey @"kAirFloatDAAPItemNameKey"
#define kAirFloatDAAPItemArtistNameKey @"kAirFloatDAAPItemArtistNameKey"
#define kAirFloatDAAPItemAlbumNameKey @"kAirFloatDAAPItemAlbumNameKey"
#define kAirFloatDAAPItemArtworkKey @"kAirFloatDAAPItemArtworkKey"

#define AirFloatDAAPClientDidUpdatePlaylistNotification @"AirFloatDAAPClientDidUpdatePlaylistNotification"

#define AirFloatDAAPPlaySpecificNotification @"AirFloatDAAPPlaySpecificNotification"
#define kAirFloatDAAPPlaySpecificIdentifierKey @"kAirFloatDAAPPlaySpecificIdentifierKey"

@class AirFloatDAAPClient;

@protocol AirFloatDAAPClientDelegate

- (void)loginFailedInDaapClientAndCannotFallbackToDacp:(AirFloatDAAPClient*)client;

@end

@interface AirFloatDAAPClient : NSObject <NSNetServiceBrowserDelegate, NSNetServiceDelegate> {
    
    NSString* _dacpId;
    NSString* _activeRemove;
    
    NSString* _host;
    
    NSMutableDictionary* _nowPlaying;
    NSMutableArray* _databases;
    
    AirFloatDAAPClientType _clientType;
    
    NSUInteger _session;
    
    id<AirFloatDAAPClientDelegate> _delegate;
    
    dispatch_queue_t _clientQueue;
    
}

+ (NSData*)addService:(NSString*)service;
+ (NSData*)guidForService:(NSString*)service;
+ (void)removeServiceForGuid:(NSData*)guid;
+ (BOOL)hasPairedServices;

+ (NSString*)daapServiceTypeForClientUserAgent:(NSString*)userAgent;

@property (nonatomic,assign) id<AirFloatDAAPClientDelegate> delegate;
@property (nonatomic,readonly) AirFloatDAAPClientType clientType;

- (id)initWithHost:(NSString*)host dacpId:(NSString*)dacpId activeRemove:(NSString*)activeRemove andServiceName:(NSString*)serviceName;

@end

//
//  AirFloatServerController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RAOPServer.h"

#define AirFloatServerStartedNotification @"AirFloatServerStartedNotification"
#define AirFloatServerStoppedNotification @"AirFloatServerStartedNotification"

#define AirFloatClientSupportsPlayControlsNotification @"AirFloatClientSupportsPlayControlsNotification"
#define AirFloatClientConnectedNotification @"AirFloatClientConnectedNotification"
#define AirFloatClientDisconnectedNotification @"AirFloatClientDisconnectedNotification"
#define AirFloatClientStartedRecordingNotification @"AirFloatClientStartedRecordingNotification"
#define AirFloatClientStoppedRecordingNotification @"AirFloatClientStoppedRecordingNotification"
#define AirFloatClientUpdatedMetadataNotification @"AirFloatClientUpdatedMetadataNotification"

#define kAirFloatClientMetadataData @"kAirFloatClientMetadataData"
#define kAirFloatClientMetadataArtistName @"kAirFloatClientMetadataArtist"
#define kAirFloatClientMetadataTrackTitle @"kAirFloatClientMetadataTitle"
#define kAirFloatClientMetadataAlbum @"kAirFloatClientMetadataAlbum"
#define kAirFloatClientMetadataContentType @"kAirFloatClientMetadataContentType"

@interface AirFloatServerController : NSObject <NSNetServiceBrowserDelegate, NSNetServiceDelegate> {
    
    NSRunLoop* _netServicesRunLoop;
    NSNetService* _netService;
    RAOPServer* _server;
    
    NSString* _serverMacAddress;
    int _serverPort;
    
    NSNetServiceBrowser* _netServiceBrowser;

    RAOPConnection* _currentConnection;
    NSString* _dacpHost;
    int _dacpPort;
    
    BOOL _isWifiAvailable;
    
}

@property (nonatomic, readonly) BOOL isWifiAvailable;
@property (nonatomic, readonly) BOOL isRunning;
@property (nonatomic, readonly) BOOL hasClientConnected;

- (BOOL)start;
- (void)stop;

- (void)dacpNext;
- (void)dacpPlay;
- (void)dacpPrev;

@end

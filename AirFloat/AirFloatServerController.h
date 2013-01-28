//
//  AirFloatServerController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/19/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "AirFloatDAAPClient.h"
#import "AirFloatBonjourBrowser.h"
#import "AirFloatNotificationsHub.h"
#import "AirFloatInterfaces.h"
#import "AirFloatReachability.h"
#import "AirFloatBonjourController.h"

typedef enum {
    
    kAirFloatServerControllerStatusUnknown = 0,
    kAirFloatServerControllerStatusNeedsWifi,
    kAirFloatServerControllerStatusReady,
    kAirFloatServerControllerStatusReceiving
    
} AirFloatServerControllerStatus;

#define AirFloatServerControllerDidChangeStatusNotification @"AirFloatServerControllerDidChangeStatusNotification"
#define AirFloatServerControllerFailedFindingDAAPNoification @"AirFloatServerControllerFailedFindingDAAPNoification"

@interface AirFloatServerController : NSObject <NSNetServiceBrowserDelegate, NSNetServiceDelegate, AirFloatReachabilityDelegate, AirFloatBonjourBrowserDelegate, AirFloatDAAPClientDelegate, AVAudioSessionDelegate> {
    
    void* _server;
    
    AirFloatBonjourController* _bonjour;
    AirFloatReachability* _wifiReachability;
    AirFloatBonjourBrowser* _bonjourBrowser;
    AirFloatDAAPClient* _daapClient;
    NSArray* _currentDAAPAddresses;
    
    AirFloatNotificationsHub* _notificationHub;
    
    NSDate* _lastLocalhostErrorNoticationDate;
    
    UIBackgroundTaskIdentifier _suspendBackgroundTaskIdentifier;
    
}

@property (nonatomic,readonly) AirFloatReachability* wifiReachability;
@property (nonatomic,readonly) AirFloatServerControllerStatus status;
@property (nonatomic,readonly) NSString* connectedHost;
@property (nonatomic,readonly) NSString* connectedUserAgent;
@property (nonatomic,readonly) NSArray* currentDAAPAddresses;

+ (AirFloatServerController*)sharedServerController;

- (void)start;
- (void)stop;

@end

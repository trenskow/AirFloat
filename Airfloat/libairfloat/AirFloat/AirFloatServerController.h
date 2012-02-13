//
//  AirFloatServerController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AirFloatDACPClient.h"
#import "AirFloatDACPBrowser.h"
#import "AirFloatNotificationsHub.h"
#import "AirFloatInterfaces.h"
#import "AirFloatReachability.h"
#import "AirFloatBonjourController.h"
#import "RAOPServer.h"

#define AirFloatServerControllerDidChangeStatus @"AirFloatServerControllerDidChangeStatus"

@interface AirFloatServerController : NSObject <NSNetServiceBrowserDelegate, NSNetServiceDelegate, AirFloatReachabilityDelegate, AirFloatDACPBrowserDelegate> {
    
    RAOPServer* _server;
    
    AirFloatBonjourController* _bonjour;
    AirFloatReachability* _wifiReachability;
    AirFloatDACPBrowser* _dacpBrowser;
    AirFloatDACPClient* _dacpClient;
    
    AirFloatNotificationsHub* _notificationHub;
    
    uint32_t _connectionCount;
    BOOL _recording;
    
}

@property (readonly) AirFloatReachability* wifiReachability;
@property (readonly) BOOL isRunning;
@property (readonly) BOOL hasClientConnected;
@property (readonly) BOOL isRecording;

- (void)start;
- (void)stop;

@end

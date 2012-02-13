//
//  AirFloatDACPClient.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

enum AirFloatPlaybackStatus {
    
    kAirFloatPlaybackStatusUnknown = 0,
    kAirFloatPlaybackStatusPaused = 2,
    kAirFloatPlaybackStatusPlaying = 4
    
};

#define AirFloatPlaybackStatusUpdatedNotification @"AirFloatPlaybackStatusUpdatedNotification"

#define kAirFloatPlaybackStatusKey @"kAirFloatPlaybackStatusKey"

#define AirFloatPlaybackPlayPauseNotification @"AirFloatPlaybackPlayPauseNotification"
#define AirFloatPlaybackNextNotification @"AirFloatPlaybackNextNotification"
#define AirFloatPlaybackPrevNotification @"AirFloatPlaybackPrevNotification"

@interface AirFloatDACPClient : NSObject <NSNetServiceBrowserDelegate, NSNetServiceDelegate> {
    
    NSString* _dacpId;
    NSString* _activeRemove;
    
    NSString* _host;
    
    dispatch_queue_t _clientQueue;
    
}

- (id)initWithHost:(NSString*)host dacpId:(NSString*)dacpId andActiveRemove:(NSString*)activeRemove;

@end

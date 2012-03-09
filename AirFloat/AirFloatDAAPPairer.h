//
//  AirFloatDAAPPairer.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AirFloatWebServer.h"

#define AirFloatDAAPPairerDidPairNotification @"AirFloatDAAPPairerDidPairNotification"
#define kAirFloatDAAPPairerGuidKey @"kAirFloatDAAPPairerGuidKey"

@class AirFloatDAAPPairer;

@protocol AirFloatDAAPPairerDelegate

- (void)pairerDidAuthenticate:(AirFloatDAAPPairer*)pairer;
- (void)pairerClientWasRejected:(AirFloatDAAPPairer*)pairer;

@end

@interface AirFloatDAAPPairer : NSObject <AirFloatWebServerDelegate, AirFloatWebConnectionDelegate> {
    
    NSNetService* _netService;
    NSString* _passcode;
    
    NSString* _expectedResponse;
    
    AirFloatWebServer* _webServer;
    
    NSData* _loginToken;
    
    __weak id<AirFloatDAAPPairerDelegate> _delegate;
    
}

@property (assign, nonatomic) id<AirFloatDAAPPairerDelegate> delegate;

- (id)initWithPasscode:(NSString*)passcode;

@end

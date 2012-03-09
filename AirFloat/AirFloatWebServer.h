//
//  AirFloatWebServer.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AirFloatWebConnection.h"

@class AirFloatWebServer;
@class AirFloatSocketEndPoint;

@protocol AirFloatWebServerDelegate

- (BOOL)server:(AirFloatWebServer*)server shouldAcceptConnection:(AirFloatWebConnection*)connection;

@end

@interface AirFloatWebServer : NSObject {
    
    void* _server;
    
    id<AirFloatWebServerDelegate> _delegate;
    
}

@property (nonatomic,assign) id<AirFloatWebServerDelegate> delegate;
@property (nonatomic,readonly) BOOL isRunning;
@property (nonatomic,readonly) NSUInteger connectionCount;
@property (nonatomic,readonly) AirFloatSocketEndPoint* localEndPoint;

- (BOOL)startServerOnPort:(NSUInteger)port tryPorts:(NSUInteger)portRange;
- (BOOL)startServerOnPort:(NSUInteger)port;

- (void)stopServer;

@end

//
//  AirFloatWebConnection.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AirFloatSocketEndPoint.h"
#import "AirFloatWebRequest.h"

@class AirFloatWebConnection;

@protocol AirFloatWebConnectionDelegate

- (void)processRequest:(AirFloatWebRequest*)request fromConnection:(AirFloatWebConnection*)connection;
- (void)connectionDidClose:(AirFloatWebConnection*)connection;

@end

@interface AirFloatWebConnection : NSObject {
    
    __weak void* _connection;
    __weak id<AirFloatWebConnectionDelegate> _delegate;
    
}

@property (nonatomic,assign) id<AirFloatWebConnectionDelegate> delegate;
@property (nonatomic,readonly) AirFloatSocketEndPoint* localEndPoint;
@property (nonatomic,readonly) AirFloatSocketEndPoint* remoteEndPoint;
@property (nonatomic,readonly) BOOL isConnected;

- (void)closeConnection;

@end

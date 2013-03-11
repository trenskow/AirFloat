//
//  AirFloatWebConnection.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "webconnection.h"

#import "AirFloatSocketEndPoint+Private.h"
#import "AirFloatWebRequest+Private.h"
#import "AirFloatWebConnection+Private.h"

static void processRequestCallbackHelper(web_connection_p connection, web_request_p request, void* ctx) {
    
    @autoreleasepool {
        [(AirFloatWebConnection*)ctx _processRequest:request];
    }
    
}

static void connectionClosedCallbackHelper(web_connection_p connection, void* ctx) {
    
    @autoreleasepool {
        [(AirFloatWebConnection*)ctx _connectionClosed];
    }
    
}

@implementation AirFloatWebConnection

@synthesize delegate=_delegate;

#pragma mark - Allocation / Deallocation

- (id)_initWithConnection:(id)connection {
    
    if ((self = [super init])) {
        
        _connection = connection;
        web_connection_set_request_callback(self._connection, processRequestCallbackHelper, self);
        web_connection_set_closed_callback(self._connection, connectionClosedCallbackHelper, self);
                
    }
    
    return self;
    
}

#pragma mark - Private Properties

- (web_connection_p)_connection {
    
    return (web_connection_p)_connection;
    
}

#pragma mark - Public Properties

- (AirFloatSocketEndPoint*)localEndPoint {
    
    return [[[AirFloatSocketEndPoint alloc] _initWithEndPoint:web_connection_get_local_end_point(self._connection)] autorelease];
    
}

- (AirFloatSocketEndPoint*)remoteEndPoint {
    
    return [[[AirFloatSocketEndPoint alloc] _initWithEndPoint:web_connection_get_remote_end_point(self._connection)] autorelease];
    
}

- (BOOL)isConnected {
    
    return web_connection_is_connected(self._connection);
    
}

#pragma mark - Public Methods

- (void)closeConnection {
    
    web_connection_close(self._connection);
    web_connection_wait_close(self._connection);
    
}

#pragma mark - Private Methods

- (void)_processRequest:(web_request_p)request {
    
    [self.delegate processRequest:[[[AirFloatWebRequest alloc] _initWithWebRequest:request forConnection:self] autorelease] fromConnection:self];
    
}

- (void)_connectionClosed {
    
    [self.delegate connectionDidClose:self];
    
    // These are wrapper classes that does not have a owner.
    // WebConnection is destroyed upon connection close. So is this.
    [self release];
    
}

@end

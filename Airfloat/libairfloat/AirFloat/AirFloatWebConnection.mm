//
//  AirFloatWebConnection.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "WebConnection.h"
#import "AirFloatSocketEndPoint+Private.h"
#import "AirFloatWebRequest+Private.h"
#import "AirFloatWebConnection+Private.h"

class WebRequest;

static void processRequestCallbackHelper(WebConnection* connection, WebRequest* request, void* ctx) {
    
    @autoreleasepool {
        [(AirFloatWebConnection*)ctx _processRequest:request];
    }
    
}

static void connectionClosedCallbackHelper(WebConnection* connection, void* ctx) {
    
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
        self._connection->setCallbackCtx(self);
        self._connection->setProcessRequestCallback(processRequestCallbackHelper);
        self._connection->setConnectionClosed(connectionClosedCallbackHelper);
        
    }
    
    return self;
    
}

#pragma mark - Private Properties

- (WebConnection*)_connection {
    
    return (WebConnection*)_connection;
    
}

#pragma mark - Public Properties

- (AirFloatSocketEndPoint*)localEndPoint {
    
    return [[[AirFloatSocketEndPoint alloc] _initWithEndPoint:self._connection->getLocalEndPoint()] autorelease];
    
}

- (AirFloatSocketEndPoint*)remoteEndPoint {
    
    return [[[AirFloatSocketEndPoint alloc] _initWithEndPoint:self._connection->getRemoteEndPoint()] autorelease];
    
}

- (BOOL)isConnected {
    
    return self._connection->isConnected();
    
}

#pragma mark - Public Methods

- (void)closeConnection {
    
    self._connection->closeConnection();
    self._connection->waitConnection();
    
}

#pragma mark - Private Methods

- (void)_processRequest:(WebRequest *)request {
    
    [self.delegate processRequest:[[[AirFloatWebRequest alloc] _initWithWebRequest:request forConnection:self] autorelease] fromConnection:self];
    
}

- (void)_connectionClosed {
    
    [self.delegate connectionDidClose:self];
    
    // These are wrapper classes that does not have a owner.
    // WebConnection is destroyed upon connection close. So is this.
    [self release];
    
}

@end

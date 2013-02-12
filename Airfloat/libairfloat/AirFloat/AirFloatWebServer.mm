//
//  AirFloatWebServer.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "Server.h"
#import "AirFloatWebConnection+Private.h"
#import "AirFloatSocketEndPoint+Private.h"
#import "AirFloatWebServer.h"

@interface AirFloatWebServer (Private)

@property (nonatomic,readonly) WebServer* _server;

- (BOOL)_acceptConnection:(WebConnection*) connection;

@end

static bool acceptConnectionCallback(WebServer* server, WebConnection* connection, void* ctx) {
    
    @autoreleasepool {
        return [(AirFloatWebServer*)ctx _acceptConnection:connection];
    }
    
}

@implementation AirFloatWebServer

@synthesize delegate=_delegate;

#pragma mark - Allocation / Deallocation

- (id)init {
    
    if ((self = [super init])) {
        _server = new WebServer((SocketEndPointType)(kSocketEndPointTypeIPv4 | kSocketEndPointTypeIPv6));
        self._server->setAcceptConnectionCallback(acceptConnectionCallback, self);
    }
    
    return self;
    
}

- (void)dealloc {
    
    delete (WebServer*)self._server;
    
    [super dealloc];
    
}

#pragma mark - Public Methods

- (BOOL)startServerOnPort:(NSUInteger)port tryPorts:(NSUInteger)portRange {
    
    return self._server->startServer(port, portRange);
    
}

- (BOOL)startServerOnPort:(NSUInteger)port {
    
    return [self startServerOnPort:port tryPorts:1];
    
}

- (void)stopServer {
    
    self._server->stopServer();
    self._server->waitServer();
    
}

#pragma mark - Properties

- (BOOL)isRunning {
    
    return self._server->isRunning();
    
}

- (NSUInteger)connectionCount {
    
    return self._server->getConnectionCount();
    
}

- (AirFloatSocketEndPoint*)localEndPoint {
    
    return [[[AirFloatSocketEndPoint alloc] _initWithEndPoint:self._server->getLocalEndPoint()] autorelease];
    
}

#pragma mark - Private Properties

- (WebServer*)_server {
    
    return (WebServer*)_server;
    
}

#pragma mark - Private Methods

- (BOOL)_acceptConnection:(WebConnection *)connection {
    
    AirFloatWebConnection* afConnection = [[AirFloatWebConnection alloc] _initWithConnection:connection];
    BOOL ret = [self.delegate server:self shouldAcceptConnection:afConnection];
    if (!ret)
        [afConnection release];
    
    return ret;
    
}

@end

//
//  AirFloatWebServer.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatWebConnection+Private.h"
#import "AirFloatSocketEndPoint+Private.h"
#import "AirFloatWebServer.h"

@interface AirFloatWebServer (Private)

@property (nonatomic,readonly) web_server_p _server;

- (BOOL)_acceptConnection:(web_connection_p)connection;

@end

static bool acceptConnectionCallback(web_server_p server, web_connection_p connection, void* ctx) {
    
    @autoreleasepool {
        return [(AirFloatWebServer*)ctx _acceptConnection:connection];
    }
    
}

@implementation AirFloatWebServer

@synthesize delegate=_delegate;

#pragma mark - Allocation / Deallocation

- (id)init {
    
    if ((self = [super init])) {
        _server = web_server_create((sockaddr_type)(sockaddr_type_inet_4 | sockaddr_type_inet_6));
        web_server_set_accept_callback(self._server, acceptConnectionCallback, self);
    }
    
    return self;
    
}

- (void)dealloc {
    
    web_server_destroy(self._server);
    
    [super dealloc];
    
}

#pragma mark - Public Methods

- (BOOL)startServerOnPort:(NSUInteger)port tryPorts:(NSUInteger)portRange {
    
    return web_server_start(self._server, port, portRange);
    
}

- (BOOL)startServerOnPort:(NSUInteger)port {
    
    return [self startServerOnPort:port tryPorts:1];
    
}

- (void)stopServer {
    
    web_server_stop(self._server);
    web_server_wait_stop(self._server);
    
}

#pragma mark - Properties

- (BOOL)isRunning {
    
    return web_server_is_running(self._server);
    
}

- (NSUInteger)connectionCount {
    
    return web_server_get_connection_count(self._server);
    
}

- (AirFloatSocketEndPoint*)localEndPoint {
    
    return [[[AirFloatSocketEndPoint alloc] _initWithEndPoint:web_server_get_local_end_point(self._server, sockaddr_type_inet_4)] autorelease];
    
}

#pragma mark - Private Properties

- (web_server_p)_server {
    
    return (web_server_p)_server;
    
}

#pragma mark - Private Methods

- (BOOL)_acceptConnection:(web_connection_p)connection {
    
    AirFloatWebConnection* afConnection = [[AirFloatWebConnection alloc] _initWithConnection:connection];
    BOOL ret = [self.delegate server:self shouldAcceptConnection:afConnection];
    if (!ret)
        [afConnection release];
    
    return ret;
    
}

@end

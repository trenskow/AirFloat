//
//  AirFloatServerController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RTPReceiver.h"
#import "RAOPConnection.h"

#import "AirFloatServerController.h"

@interface  AirFloatServerController (Private)

- (void)_updateServerStatus;
- (void)_recordingStarted;
- (void)_recordingEnded;
- (void)_clientConnected;
- (void)_clientDisconnected;

@end

@implementation AirFloatServerController

@synthesize wifiReachability=_wifiReachability;

- (id)init {
    
    if ((self = [super init])) {
        _wifiReachability = [[AirFloatReachability wifiReachability] retain];
        _wifiReachability.delegate = self;
        
        _dacpBrowser = [[AirFloatDACPBrowser alloc] init];
        _dacpBrowser.delegate = self;
        
        _notificationHub = [[AirFloatNotificationsHub alloc] init];
    }
    
    return self;
    
}

- (void)dealloc {
    
    [_bonjour release];
    [_dacpBrowser release];
    [_notificationHub release];
    [_wifiReachability release];
    
    [super dealloc];
    
}

- (void)start {
    
    if (_server || !_wifiReachability.isAvailable)
        return;
    
    for (NSInteger port = 5000 ; port < 5010 ; port++) {
        
        _server = new RAOPServer(port);
        
        if (_server->startServer()) {
            
            [_bonjour release];
            _bonjour = [[AirFloatBonjourController alloc] initWithMacAddress:[[AirFloatInterfaces wifiInterface] objectForKey:@"mac"] andPort:port];
            
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_recordingStarted) name:AirFloatRecordingStartedNotification object:nil];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_recordingEnded) name:AirFloatRecordingEndedNotification object:nil];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_clientConnected) name:AirFloatClientConnectedNotification object:nil];
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_clientDisconnected) name:AirFloatClientDisconnectedNotification object:nil];
            
            [self _updateServerStatus];
            
            return;
            
        }
        
        delete _server;
        _server = NULL;
        
    }
    
}

- (void)stop {
    
    if (_server && RTPReceiver::isAvailable()) {
        
        _server->stopServer();
        delete _server;
        _server = NULL;
        _connectionCount = 0;
        
        [_bonjour release];
        _bonjour = nil;
        
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        
        [self _updateServerStatus];
                
    }
    
}

- (BOOL)isRunning {
    
    return (_server != NULL);
    
}

- (BOOL)hasClientConnected {
    
    return (self.isRunning && (_connectionCount > 0));
    
}

- (BOOL)isRecording {
    
    return (self.hasClientConnected && _recording);
    
}

#pragma mark DACP Browser Delegate Methods

- (void)dacpBrowser:(AirFloatDACPBrowser *)browser didFindHost:(NSString *)host {
    
    [_dacpClient release];
    _dacpClient = [[AirFloatDACPClient alloc] initWithHost:host
                                                    dacpId:[NSString stringWithCString:RTPReceiver::getStreamingReceiver()->getConnection()->getDacpId() encoding:NSUTF8StringEncoding]
                                           andActiveRemove:[NSString stringWithCString:RTPReceiver::getStreamingReceiver()->getConnection()->getActiveRemote() encoding:NSUTF8StringEncoding]];
    
}

#pragma mark Reachability Delegate Methods

- (void)reachability:(AirFloatReachability *)sender didChangeStatus:(BOOL)reachable {
    
    if (reachable && !_server)
        [self start];
    else if (!reachable && _server)
        [self stop];
    
}

#pragma mark Private Methods

- (void)_updateServerStatus {
    
    [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatServerControllerDidChangeStatus object:self];
    
}

- (void)_recordingStarted {
    
    NSString* identifier = [NSString stringWithCString:RTPReceiver::getStreamingReceiver()->getConnection()->getDacpId() encoding:NSUTF8StringEncoding];
    [_dacpBrowser findServerForIdentifier:identifier];
    
    _recording = YES;
    [self _updateServerStatus];
    
}

- (void)_recordingEnded {
    
    _recording = NO;
    [self _updateServerStatus];
    
}

- (void)_clientConnected {
    
    _connectionCount++;
    [self _updateServerStatus];
    
}

- (void)_clientDisconnected {
    
    if (_connectionCount == 0) {
        [_dacpClient release];
        _dacpClient = nil;
    }
    
    _connectionCount--;
    [self _updateServerStatus];
    
}

@end

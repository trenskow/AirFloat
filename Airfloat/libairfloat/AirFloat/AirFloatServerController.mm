//
//  AirFloatServerController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/19/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import "RTPReceiver.h"
#import "RAOPConnection.h"
#import "RAOPServer.h"

#import "AirFloatAppDelegate.h"
#import "AirFloatAdditions.h"
#import "AirFloatDAAPPairer.h"
#import "AirFloatServerController.h"

@interface  AirFloatServerController (Private)

@property (nonatomic,readonly) RAOPServer* _server;

- (void)_didPairDaap:(NSNotification*)notification;
- (void)_daapAuthenticationFailed:(NSNotification*)notification;
- (void)_localhostConnectedError;
- (void)_updateServerStatus;
- (void)_clientConnected;
- (void)_clientDisconnected;
- (void)_recordingStarted;

@end

@implementation AirFloatServerController

#pragma mark - Class Methods

+ (AirFloatServerController*)sharedServerController {
    
    return ((AirFloatAppDelegate*)[UIApplication sharedApplication].delegate).serverController;
    
}

#pragma mark - Allocation / Deallocation

- (id)init {
    
    if ((self = [super init])) {
        _wifiReachability = [[AirFloatReachability wifiReachability] retain];
        _wifiReachability.delegate = self;
        
        _bonjourBrowser = [[AirFloatBonjourBrowser alloc] init];
        _bonjourBrowser.delegate = self;
        
        _notificationHub = [[AirFloatNotificationsHub alloc] init];
    }
    
    return self;
    
}

- (void)dealloc {
    
    [self stop];
    
    [_currentDAAPAddresses release];
    [_daapClient release];
    [_lastLocalhostErrorNoticationDate release];    
    [_bonjourBrowser release];
    [_notificationHub release];
    [_wifiReachability release];
    
    [super dealloc];
    
}

#pragma mark - Public Methods

- (void)start {
    
    if (_server || !_wifiReachability.isAvailable)
        return;
    
    _server = new RAOPServer(5000);
    
    if (self._server->startServer()) {
        
        [_bonjour release];
        _bonjour = [[AirFloatBonjourController alloc] initWithMacAddress:[[AirFloatInterfaces wifiInterface] objectForKey:@"mac"] andPort:self._server->getLocalEndPoint()->getPort()];
        
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_clientConnected) name:AirFloatClientConnectedNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_clientDisconnected) name:AirFloatClientDisconnectedNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_recordingStarted) name:AirFloatRecordingStartedNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_updateServerStatus) name:AirFloatRecordingEndedNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_localhostConnectedError) name:AirFloatLocalhostConnectedErrorNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_daapAuthenticationFailed:) name:AirFloatDAAPClientFailedAuthenticationNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_didPairDaap:) name:AirFloatDAAPPairerDidPairNotification object:nil];
        
        [self _updateServerStatus];
        
        return;
    }
    
    
    delete (RAOPServer*)self._server;
    _server = NULL;
    
}

- (void)stop {
    
    if (_server && RTPReceiver::isAvailable()) {
        
        self._server->stopServer();
        self._server->waitServer();
        delete (RAOPServer*)self._server;
        _server = NULL;
        
        [_bonjour release];
        _bonjour = nil;
        
        [NSDefaultNotificationCenter removeObserver:self];
        
        [self _updateServerStatus];
                
    }
    
}

#pragma mark - Public Properties

@synthesize wifiReachability=_wifiReachability;
@synthesize currentDAAPAddresses=_currentDAAPAddresses;

- (AirFloatServerControllerStatus)status {
    
    if (_server != NULL && RTPReceiver::getStreamingReceiver() != NULL && RTPReceiver::getStreamingReceiver()->getConnection()->isConnected())
        return kAirFloatServerControllerReceivingStatus;
    else if (_server != NULL)
        return kAirFloatServerControllerReadyStatus;
    
    return kAirFloatServerControllerNeedsWifiStatus;    
    
}

- (NSString*)connectedHost {
    
    if (self.status == kAirFloatServerControllerReceivingStatus) {
        
        char ip[100];
        RTPReceiver::getStreamingReceiver()->getConnection()->getRemoteEndPoint()->getHost(ip, 100);
        return [NSString stringWithCString:ip encoding:NSASCIIStringEncoding];
        
    }
    
    return nil;
    
}

- (NSString*)connectedUserAgent {
    
    if (self.status == kAirFloatServerControllerReceivingStatus)
        return [NSString stringWithCString:RTPReceiver::getStreamingReceiver()->getConnection()->getUserAgent() encoding:NSUTF8StringEncoding];
    
    return nil;
    
}

#pragma mark - Bonjour Browser Delegate Methods

- (bool)bonjourBrowser:(AirFloatBonjourBrowser *)browser didFindAddresses:(NSArray *)addresses forService:(NSNetService*)service {
    
    NSDLog(@"Found service of type %@", [service type]);
    
    RTPReceiver* receiver;
    if ((receiver = RTPReceiver::getStreamingReceiver()) != NULL) {
        
        RAOPConnection* connection = receiver->getConnection();
        
        for (NSData* addrData in addresses) {
            
            struct sockaddr* addr = (struct sockaddr*) [addrData bytes];
            if (receiver->getConnection()->getRemoteEndPoint()->compareWithAddress(addr)) {
                
                NSDLog(@"Found %@", [service type]);
                
                if (([[service type] isEqualToString:@"_dacp._tcp."] && [[service name] isEqualToString:[NSString stringWithFormat:@"iTunes_Ctrl_%@", [NSString stringWithCString:connection->getDacpId() encoding:NSASCIIStringEncoding]]]) || [[service type] isEqualToString:[AirFloatDAAPClient daapServiceTypeForClientUserAgent:self.connectedUserAgent]]) {
                    
                    NSData* MID = [[NSNetService dictionaryFromTXTRecordData:[service TXTRecordData]] objectForKey:@"MID"];
                    NSString* serviceName = nil;
                    if (MID)
                        serviceName = [[[[NSString alloc] initWithData:MID encoding:NSASCIIStringEncoding] autorelease] substringFromIndex:2];
                    
                    if (serviceName && ![AirFloatDAAPClient guidForService:serviceName])
                        return false;
                    
                    [_currentDAAPAddresses release];
                    _currentDAAPAddresses = [addresses retain];
                    [_daapClient release];
                    _daapClient = [[AirFloatDAAPClient alloc] initWithHost:[NSString stringWithFormat:@"%@:%d", [service hostName], [service port]]
                                                                    dacpId:[NSString stringWithCString:connection->getDacpId() encoding:NSUTF8StringEncoding]
                                                              activeRemove:[NSString stringWithCString:connection->getActiveRemote() encoding:NSUTF8StringEncoding]
                                                            andServiceName:serviceName];
                    _daapClient.delegate = self;
                    
                    NSDLog(@"Connected to service %@", [service name]);
                    
                    return true;
                    
                }
            }
            
        }
        
    }
    
    return false;

}

- (void)bonjourBrowser:(AirFloatBonjourBrowser *)browser endedSearchForServiceType:(NSString *)serviceType {
    
    NSDLog(@"Search ended for service type %@", serviceType);
    if ([serviceType isEqualToString:[AirFloatDAAPClient daapServiceTypeForClientUserAgent:self.connectedUserAgent]])
        [_bonjourBrowser findService:@"_dacp._tcp."];
    else
        [NSDefaultNotificationCenter postNotificationName:AirFloatServerControllerFailedFindingDAAPNoification object:self];
    
}

#pragma mark - Reachability Delegate Methods

- (void)reachability:(AirFloatReachability *)sender didChangeStatus:(BOOL)reachable {
    
    if (reachable && !_server)
        [self start];
    else if (!reachable && _server)
        [self stop];
    
}

#pragma mark - DAAP Client Delegate Methods

- (void)loginFailedInDaapClientAndCannotFallbackToDacp:(AirFloatDAAPClient *)client {
    
    [_daapClient release];
    _daapClient = nil;
    
    [NSDefaultNotificationCenter postNotificationName:AirFloatServerControllerFailedFindingDAAPNoification object:self];
    
}

#pragma mark - Private Methods

- (void)_didPairDaap:(NSNotification *)notification {
    
    if (!_daapClient)
        [_bonjourBrowser findService:[AirFloatDAAPClient daapServiceTypeForClientUserAgent:self.connectedUserAgent]];
    
}

- (void)_daapAuthenticationFailed:(NSNotification *)notification {
    
    [AirFloatDAAPClient removeServiceForGuid:[notification.userInfo objectForKey:kAirFloatDAAPClientGuidKey]];
    
}

- (void)_localhostConnectedError {
    
    if (!_lastLocalhostErrorNoticationDate || [[NSDate date] timeIntervalSinceDate:_lastLocalhostErrorNoticationDate] > 2.0) {
        
        UILocalNotification* localNotification = [[UILocalNotification alloc] init];
        
        localNotification.fireDate = [NSDate date];
        localNotification.alertBody = [NSString stringWithFormat:@"Cannot stream audio from the %@ itself.", [UIDevice currentDevice].model];
        localNotification.soundName = UILocalNotificationDefaultSoundName;
        
        [[UIApplication sharedApplication] presentLocalNotificationNow:localNotification];
        
        [localNotification release];
        
        [_lastLocalhostErrorNoticationDate release];
        _lastLocalhostErrorNoticationDate = [[NSDate date] retain];
        
    }
    
}

- (void)_updateServerStatus {
    
    [NSDefaultNotificationCenter postNotificationName:AirFloatServerControllerDidChangeStatusNotification object:self];
    
}

- (void)_clientConnected {
    
    [self _updateServerStatus];
    
}

- (void)_clientDisconnected {
    
    [_daapClient release];
    _daapClient = nil;
    
    [_currentDAAPAddresses release];
    _currentDAAPAddresses = nil;
    
    [self _updateServerStatus];
    
}

- (void)_recordingStarted {
    
    if (!_daapClient && AirFloatSharedAppDelegate.serverController.status == kAirFloatServerControllerReceivingStatus) {
        
        RAOPConnection* connection = RTPReceiver::getStreamingReceiver()->getConnection();
        NSString* userAgent = [NSString stringWithCString:connection->getUserAgent() encoding:NSUTF8StringEncoding];
        
        if ([userAgent rangeOfString:@"iTunes"].location != NSNotFound && [AirFloatDAAPClient hasPairedServices])
            [_bonjourBrowser findService:[AirFloatDAAPClient daapServiceTypeForClientUserAgent:self.connectedUserAgent]];
        else
            [_bonjourBrowser findService:@"_dacp._tcp."];
        
    }
    
    [self _updateServerStatus];
    
}

#pragma mark - Private Properties

- (RAOPServer*)_server {
    
    return (RAOPServer*)_server;
    
}

@end

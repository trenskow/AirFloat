//
//  AirFloatServerController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#import "AirFloatServerController.h"
#import "RAOPConnection.h"
#import "RTPReceiver.h"

#import "Log.h"

extern "C" {
    #import "AirFloatInterfaces.h"
}

@interface AirFloatServerController (PrivateMethods)

- (void)_clientStartedRecording:(RAOPConnection*)connection;
- (void)_clientDisconnected:(RAOPConnection*)connection;
- (void)_clientUpdatedMetadata:(RAOPConnection*)connection;

@end

static void _clientStartedRecording(RAOPConnection* connection, void* ctx) {
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatClientStartedRecordingNotification object:(AirFloatServerController*)ctx]];
        [(AirFloatServerController*)ctx _clientStartedRecording:connection];
    });
    
}

static void _clientStoppedRecording(RAOPConnection* connection, void* ctx) {
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatClientStoppedRecordingNotification object:(AirFloatServerController*)ctx]];
    });
    
}
static void _clientDisconnected(RAOPConnection* connection, void* ctx) {
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatClientDisconnectedNotification object:(AirFloatServerController*)ctx]];
        [(AirFloatServerController*)ctx _clientDisconnected:connection];
    });
    
}

static NSString* stringFromFrame(char* buf, int size) {
    
    char ret[size+1];
    memcpy(ret, buf, size);
    ret[size] = '\0';
    
    return [NSString stringWithCString:ret encoding:NSUTF8StringEncoding];
    
}

static void _clientUpdatedMetadata(RAOPConnection* connection, void* buffer, int size, const char* contentType, void* ctx) {
    
    AirFloatServerController* serverController = (AirFloatServerController*)ctx;
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    NSData* data = [NSData dataWithBytes:buffer length:size];
    NSString* cType = [[[NSString alloc] initWithCString:contentType encoding:NSUTF8StringEncoding] autorelease];
    NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithObjectsAndKeys:data, kAirFloatClientMetadataData, cType, kAirFloatClientMetadataContentType, nil];
    if ([cType isEqualToString:@"application/x-dmap-tagged"]) {
        
        int length = size - 8;
        if (size > 0) {
            char* mBuffer = &((char*)buffer)[8];
            
            while (length > 0) {
                
                char* name = mBuffer;
                int frameSize = htonl(*((uint32_t*)&mBuffer[4]));
                mBuffer += 8;
                length -= 8;
                
                if (0 == memcmp(name, "asal", 4)) // Album
                    [dict setObject:stringFromFrame(mBuffer, frameSize) forKey:kAirFloatClientMetadataAlbum];
                else if (0 == memcmp(name, "asar", 4)) // Artist
                    [dict setObject:stringFromFrame(mBuffer, frameSize) forKey:kAirFloatClientMetadataArtistName];
                else if (0 == memcmp(name, "minm", 4)) // Track
                    [dict setObject:stringFromFrame(mBuffer, frameSize) forKey:kAirFloatClientMetadataTrackTitle];
                    
                mBuffer += frameSize;
                length -= frameSize;
                
            }
            
        }        
        
    }
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        [serverController  _clientUpdatedMetadata:connection];
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatClientUpdatedMetadataNotification object:serverController userInfo:dict]];
    });
    
    [pool release];
    
}

static void _clientConnected(RAOPServer* server, RAOPConnection* connection, void* ctx) {
    
    connection->SetCallbacksContext(ctx);
    connection->SetRecordingStartedClbk(_clientStartedRecording);
    connection->SetClientDisconnectedClbk(_clientDisconnected);
    connection->SetClientUpdatedMetadataClbk(_clientUpdatedMetadata);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatClientConnectedNotification object:(AirFloatServerController*)ctx]];
    });
    
}

@implementation AirFloatServerController

@synthesize isWifiAvailable=_isWifiAvailable;

- (id)init {
    
    if ((self = [super init])) {
        _netServiceBrowser = [[NSNetServiceBrowser alloc] init];
        [_netServiceBrowser setDelegate:self];
    }
    
    return self;
    
}

- (void)_cleanUpClientData {
    
    if (_currentConnection != NULL)
        [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatClientDisconnectedNotification object:self]];
    
    _currentConnection = NULL;
    [_dacpHost release];
    _dacpHost = nil;
    _dacpPort = 0;
    
}

- (void)dealloc {
    
    [self stop];
    [_netServiceBrowser release];
    [self _cleanUpClientData];
    
    [super dealloc];
    
}

- (void)updateBonjour {
        
     if (_serverMacAddress && _serverPort) {
        
        if (!_netService)
            _netService = [[NSNetService alloc] initWithDomain:@"local." type:@"_raop._tcp." name:[NSString stringWithFormat:@"%@@%@", _serverMacAddress, [UIDevice currentDevice].name] port:_serverPort];
         
        [_netService stop];
        
        NSDictionary* txtRecord = [NSDictionary dictionaryWithObjectsAndKeys:
                                   @"1", @"txtvers",
                                   @"0,1", @"et",
                                   @"1", @"ek",
                                   @"16", @"ss",
                                   @"44100", @"sr",
                                   @"TCP,UDP", @"tp",
                                   @"0,1", @"cn",
                                   @"true", @"da",
                                   @"0x4", @"sf",
                                   @"65537", @"vn",
                                   @"0,1,2", @"md",
                                   @"104.29", @"vs",
                                   @"false", @"sv",
                                   @"false", @"sm",
                                   @"2", @"ch",
                                   @"44100", @"sr",
                                   ([[[NSUserDefaults standardUserDefaults] objectForKey:@"pw"] length] > 0 ? @"true" : @"false"), @"pw",
                                   nil];
        
        [_netService setTXTRecordData:[NSNetService dataFromTXTRecordDictionary:txtRecord]];
        
        [_netService publish];
        
    } else if (_netService)
        [_netService stop];
    
}

- (BOOL)start {
    
    if (!_server) {
        
        NSDictionary* ifs = allInterfaces();
        NSString* interface = nil;
        
        for (NSString* key in [ifs allKeys]) {
            
            if ([key length] >= 3 && [[key substringToIndex:2] isEqualToString:@"en"] &&
                [[key substringFromIndex:2] integerValue] < 3 &&
                [[ifs objectForKey:key] objectForKey:@"address"] != nil) {
                interface = key;
                break;
            }
            
        }
        
        if ((_isWifiAvailable = (interface != nil))) {
            
            int port = 5000;        
            
            log(LOG_INFO, "Interface: %s (MAC:%s/IPv4:%s/IPv6:%s)", 
                [interface cStringUsingEncoding:NSASCIIStringEncoding], 
                [[[ifs objectForKey:interface] objectForKey:@"mac"] cStringUsingEncoding:NSASCIIStringEncoding],
                [[[ifs objectForKey:interface] objectForKey:@"address"] cStringUsingEncoding:NSASCIIStringEncoding],
                [[[ifs objectForKey:interface] objectForKey:@"address6"] cStringUsingEncoding:NSASCIIStringEncoding]);
            
            for (;;) {
                
                _server = new RAOPServer(port);
                _server->setConnectionCallback(_clientConnected, self);
                
                if (_server->startServer()) {
                    
                    _serverMacAddress = [[[ifs objectForKey:interface] objectForKey:@"mac"] retain];
                    _serverPort = port;
                    
                    [self updateBonjour];
                    
                    [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatServerStartedNotification object:self]];
                    
                    return YES;
                    
                } else
                    log(LOG_ERROR, "Unable to start server at port %d.", port);
                
                port++;
                
                if (port == 5010) {
                    log(LOG_ERROR, "Tried ten different ports. Nothing worked. Exiting");
                    break;
                }
                
            }
            
        } else
            log(LOG_ERROR, "No availabe interface found");
        
    }
    
    return NO;
    
}

- (void)stop {
    
    if (_server) {
        
        _server->stopServer();
        [_serverMacAddress release];
        _serverMacAddress = NULL;
        _serverPort = 0;
        delete _server;
        _server = NULL;
        
        [self updateBonjour];
        
    }
    
}

- (BOOL)isRunning {
    
    return (_server != NULL);
    
}

- (BOOL)hasClientConnected {
    
    return (self.isRunning && !RTPReceiver::IsAvailable());
    
}

// DACP

- (NSCachedURLResponse *)connection:(NSURLConnection *)connection willCacheResponse:(NSCachedURLResponse *)cachedResponse {
    
    return nil;
    
}

- (NSURLRequest*)_urlRequestForCommand:(NSString*)cmd {
    
    if (_dacpHost) {
        
        NSString* dacpId = [NSString stringWithCString:_currentConnection->GetDacpId() encoding:NSASCIIStringEncoding];
        NSString* activeRemote = [NSString stringWithCString:_currentConnection->GetActiveRemote() encoding:NSASCIIStringEncoding];
        NSString* sessionId = [NSString stringWithCString:_currentConnection->GetSessionId() encoding:NSASCIIStringEncoding];
        
        NSString* urlString = [NSString stringWithFormat:@"http://%@:%d/ctrl-int/1/%@?session-id=%@", _dacpHost, _dacpPort, cmd, sessionId];
        NSMutableURLRequest* urlRequest = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:urlString]];
        
        NSLog(@"DACP: URL Request: %@", urlString);

        if ([dacpId length] > 0)
            [urlRequest addValue:dacpId forHTTPHeaderField:@"Dacp-Id"];
        if ([activeRemote length] > 0)
            [urlRequest addValue:activeRemote forHTTPHeaderField:@"Active-Remote"];
        
        return urlRequest;
        
    }
    
    return nil;
    
}

- (void)_executeDacpCommand:(NSString*)cmd {
    
    [NSURLConnection connectionWithRequest:[self _urlRequestForCommand:cmd] delegate:self];
    
}

- (void)dacpNext {
    
    [self _executeDacpCommand:@"nextitem"];
    
}

- (void)dacpPlay {
    
    [self _executeDacpCommand:@"playpause"];
    
}

- (void)dacpPrev {
    
    [self _executeDacpCommand:@"previtem"];
    
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
    
    NSLog(@"DACP: Response %d", [(NSHTTPURLResponse*)response statusCode]);
    
}

- (void)_clientUpdatedMetadata:(RAOPConnection *)connection {
    
    [self _executeDacpCommand:@"playstatusupdate"];
    
}

// Delegate methods

- (void)netServiceDidResolveAddress:(NSNetService *)sender {
    
    if (_currentConnection) {
        
        for (NSData* saData in [sender addresses]) {
            
            struct sockaddr* sa = (sockaddr*)[saData bytes];
            
            struct in6_addr addr;
            memset(&addr, 0, sizeof(in6_addr));
            
            if (sa->sa_family == AF_INET)
                addr = SocketEndPoint::IPv4AddressToIPv6Address(((struct sockaddr_in*)sa)->sin_addr);
            else if (sa->sa_family == AF_INET6)
                addr = ((struct sockaddr_in6*)sa)->sin6_addr;
            
            sockaddr_in6* host = (struct sockaddr_in6*) _currentConnection->GetHost().SocketAdress();
            
            if (memcmp(&addr, &host->sin6_addr, sizeof(in6_addr)) == 0) {
                
                _dacpHost = [sender hostName];
                _dacpPort = [sender port];
                
                [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatClientSupportsPlayControlsNotification object:self]];
                
                [self _executeDacpCommand:@"playstatusupdate"];
                
                break;
                
            }
            
        }
        
    }
    
    [sender release];
    
}

- (void)netService:(NSNetService *)sender didNotResolve:(NSDictionary *)errorDict {
    
    [sender release];
    
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didFindService:(NSNetService *)netService moreComing:(BOOL)moreServicesComing {
    
    [netService retain];
    [netService setDelegate:self];
    [netService resolveWithTimeout:5.0];
    
}

// Callbacks

- (void)_clientStartedRecording:(RAOPConnection*)connection {
    
    [_netServiceBrowser searchForServicesOfType:@"_dacp._tcp." inDomain:@"local."];
    
    _currentConnection = connection;
    
}

- (void)_clientDisconnected:(RAOPConnection*)connection {
    
    [self _cleanUpClientData];
    
}

@end

//
//  AirFloatDACPClient.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "DMAPParser.h"
#import "RTPReceiver.h"
#import "RAOPConnection.h"
#import "AirFloatNotificationsHub.h"
#import "AirFloatDACPClient.h"

@interface AirFloatDACPClient (Private)

- (DMAPParser*)_executeCommand:(NSString*)command needsResponse:(BOOL)needsResponse;
- (void)_updatePlaybackStatus;
- (void)_playPause;
- (void)_next;
- (void)_prev;

@end

@implementation AirFloatDACPClient

- (id)initWithHost:(NSString *)host dacpId:(NSString *)dacpId andActiveRemove:(NSString *)activeRemove {
    
    if ((self = [self init])) {
        
        _host = [host retain];
        _dacpId = [dacpId retain];
        _activeRemove = [activeRemove retain];
        
        _clientQueue = dispatch_queue_create("com.AirFloat.DACPClientQueue", DISPATCH_QUEUE_SERIAL);
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_updatePlaybackStatus) name:AirFloatRecordingStartedNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_updatePlaybackStatus) name:AirFloatRecordingEndedNotification object:nil];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_playPause) name:AirFloatPlaybackPlayPauseNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_next) name:AirFloatPlaybackNextNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_prev) name:AirFloatPlaybackPrevNotification object:nil];
        
        [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackStatusUpdatedNotification object:self userInfo:[NSDictionary dictionaryWithObject:[NSNumber numberWithInteger:kAirFloatPlaybackStatusPlaying] forKey:kAirFloatPlaybackStatusKey]];

    }
    
    return self;
    
}

- (void)dealloc {
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];    
    
    [_host release];
    
    [_dacpId release];
    [_activeRemove release];
    
    [super dealloc];
    
}

- (DMAPParser*)_executeCommand:(NSString*)command needsResponse:(BOOL)needsResponse {
    
    NSString* urlString = [NSString stringWithFormat:@"http://%@/%@", _host, command];
    NSLog(@"DACP Requesting: %@", urlString);
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:urlString]];
    
    [request addValue:_activeRemove forHTTPHeaderField:@"Active-Remote"];
    [request addValue:_dacpId forHTTPHeaderField:@"DACP-ID"];
    
    NSLog(@"%@", [[request allHTTPHeaderFields] description]);
    
    NSHTTPURLResponse* response = nil;
    NSError* error = nil;
    NSData* data = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];
    
    if (error == nil) {
        
        NSLog(@"DACP Response code: %d", [response statusCode]);
        NSLog(@"DACP Content-Type: %@", [[response allHeaderFields] objectForKey:@"Content-Type"]);
        NSLog(@"DACP Content-Length: %lld", [response expectedContentLength]);
        
        if (data && needsResponse)
            return new DMAPParser((char*)[data bytes], [data length]);
        
    } else
        NSLog(@"%@", [error description]);
    
    return nil;
    
}

- (void)_updatePlaybackStatus {
    
    if (dispatch_get_current_queue() == dispatch_get_main_queue())
        dispatch_async(_clientQueue, ^{
            DMAPParser* playStatus = [self _executeCommand:@"ctrl-int/1/playstatusupdate" needsResponse:YES];
            
            DMAPParser* container;
            if (playStatus != NULL && (container = playStatus->containerForIdentifier("com.AirFloat.NowPlayingContainer")) != NULL) {
                
                NSNumber* playbackStatus = [NSNumber numberWithInteger:(NSInteger)container->charForIdentifier("com.AirFloat.NowPlayingStatus")];
                dispatch_async(dispatch_get_main_queue(), ^{
                    [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackStatusUpdatedNotification object:self userInfo:[NSDictionary dictionaryWithObject:playbackStatus forKey:kAirFloatPlaybackStatusKey]];
                });
                
            }
            
            delete playStatus;
            
        });
    
}

- (void)_playPause {
    
    if (dispatch_get_current_queue() == dispatch_get_main_queue())
        dispatch_async(_clientQueue, ^{
            [self _executeCommand:@"ctrl-int/1/playpause" needsResponse:NO];
        });
    
}

- (void)_next {
    
    if (dispatch_get_current_queue() == dispatch_get_main_queue())
        dispatch_async(_clientQueue, ^{
            [self _executeCommand:@"ctrl-int/1/nextitem" needsResponse:NO];
        });
    
}

- (void)_prev {
    
    if (dispatch_get_current_queue() == dispatch_get_main_queue())
        dispatch_async(_clientQueue, ^{
            [self _executeCommand:@"ctrl-int/1/previtem" needsResponse:NO];
        });
    
}

@end

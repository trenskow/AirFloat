//
//  AirFloatDAAPClient.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatDAAPPairer.h"
#import "AirFloatNotificationsHub.h"
#import "AirFloatDAAPClient.h"

#define kAirFloatDatabaseIdKey @"kAirFloatDatabaseIdKey"
#define kAirFloatDatabasePersistentIdKey @"kAirFloatDatabasePersistentIdKey"
#define kAirFloatDatabasePlaylistsKey @"kAirFloatPlaylistsKey"
#define kAirFloatPlaylistIdKey @"kAirFloatPlaylistIdKey"
#define kAirFloatPlaylistPersistentIdKey @"kAirFloatPlaylistPersistentIdKey"

#define kAirFloatDAAPItemOwnIdKey @"kAirFloatDAAPItemsOwnIdKey"

typedef struct {
    
    uint32_t databaseId;
    uint32_t containerId;
    uint32_t _unused;
    uint32_t itemId;
    
} AirFloatDAAPNowPlayingInfo;

@interface AirFloatDAAPClient (Private)

- (void)_dacpDidPair:(NSNotification*)notification;

- (NSData*)_dataForCommand:(NSString*)command statusCode:(NSInteger*)statusCode;
- (NSData*)_dataForCommand:(NSString*)command;
- (NSDictionary*)_executeCommand:(NSString*)command statusCode:(NSInteger*)statusCode;
- (NSDictionary*)_executeCommand:(NSString*)command;
- (void)_playSpecific:(NSNotification*)noticiation;
- (NSData*)_persistantIdForAtom:(NSUInteger)identifier updated:(BOOL)updated;
- (NSData*)_persistantIdForAtom:(NSUInteger)identifier;
- (void)_updateDatabases;
- (void)_login:(NSData*)guid;
- (void)_delayedUpdatePlaybackStatus;
- (void)_updatePlaybackStatus;
- (BOOL)_updateServerInfo;
- (void)_playPause;
- (void)_next;
- (void)_prev;

@end

@implementation AirFloatDAAPClient

#pragma mark - Class Methods

+ (NSData*)addService:(NSString *)service {
    
    NSMutableDictionary* services = [[[NSStandarUserDefaults objectForKey:@"AirFloatPairedClients"] mutableCopy] autorelease];
    if (!services)
        services = [NSMutableDictionary dictionary];
    
    NSData* data;
    if (!(data = [services objectForKey:service])) {
        
        data = [NSData dataWithRandomData:8];
        
        [services setObject:data forKey:[service stringByRightPaddingToLength:16 withString:@"0" startingAtIndex:0]];
        [NSStandarUserDefaults setObject:services forKey:@"AirFloatPairedClients"];
        [NSStandarUserDefaults synchronize];
        
    }
    
    return data;
    
}

+ (NSData*)guidForService:(NSString *)service {
    
    return [[NSStandarUserDefaults objectForKey:@"AirFloatPairedClients"] objectForKey:[service stringByRightPaddingToLength:16 withString:@"0" startingAtIndex:0]];
    
}

+ (void)removeServiceForGuid:(NSData *)guid {
    
    NSMutableDictionary* services = [[[NSStandarUserDefaults objectForKey:@"AirFloatPairedClients"] mutableCopy] autorelease];
    for (NSString* key in services)
        if ([[services objectForKey:key] isEqualToData:guid]) {
            [services removeObjectForKey:key];
            if ([services count] > 0)
                [NSStandarUserDefaults setObject:services forKey:@"AirFloatPairedClients"];
            else
                [NSStandarUserDefaults removeObjectForKey:@"AirFloatPairedClients"];
            [NSStandarUserDefaults synchronize];
            return;
        }
    
}

+ (BOOL)hasPairedServices {
    
    return ([[NSStandarUserDefaults objectForKey:@"AirFloatPairedClients"] count] > 0);
    
}

#pragma mark DAAP Client Identification

+ (NSString*)daapServiceTypeForClientUserAgent:(NSString*)userAgent {
    
    NSArray* components = [userAgent componentsSeparatedByString:@" "];
    
    for (NSString* component in components)        
        if ([component length] > 7 && [[component substringToIndex:7] isEqualToString:@"iTunes/"])
            if ([[component substringFromIndex:7] doubleValue] > 9.0)
                return @"_home-sharing._tcp.";
    
    return @"_home-sharing._tcp.";
    
}

#pragma mark - Allocation / Deallocation

- (id)initWithHost:(NSString*)host dacpId:(NSString*)dacpId activeRemove:(NSString*)activeRemove andServiceName:(NSString *)serviceName {
    
    if ((self = [self init])) {
        
        _host = [host retain];
        _dacpId = [dacpId retain];
        _activeRemove = [activeRemove retain];
        
        _clientQueue = dispatch_queue_create("com.AirFloat.DACPClientQueue", DISPATCH_QUEUE_SERIAL);
        
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_updatePlaybackStatus) name:AirFloatRecordingStartedNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_updatePlaybackStatus) name:AirFloatRecordingEndedNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_delayedUpdatePlaybackStatus) name:AirFloatTrackInfoUpdatedNotification object:nil];
        
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_playPause) name:AirFloatDAAPPlaybackPlayPauseNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_next) name:AirFloatDAAPPlaybackNextNotification object:nil];
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_prev) name:AirFloatDAAPPlaybackPrevNotification object:nil];
        
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_playSpecific:) name:AirFloatDAAPPlaySpecificNotification object:nil];
        
        [NSDefaultNotificationCenter addObserver:self selector:@selector(_dacpDidPair:) name:AirFloatDAAPPairerDidPairNotification object:nil];
        
        NSData* guid = nil;
        if (serviceName)
            guid = [[self class] guidForService:serviceName];
        
        if (guid)
            [self _login:guid];
        else
            [self _updateServerInfo];

    }
    
    return self;
    
}

- (void)dealloc {
    
    [NSDefaultNotificationCenter removeObserver:self];
    
    [_databases release];
    [_nowPlaying release];
    
    [_host release];
    [_dacpId release];
    [_activeRemove release];
    
    [super dealloc];
    
}

#pragma mark - Notification Handlers

- (void)_dacpDidPair:(NSNotification *)notification {
    
    // Wait one second in order for iTunes to register guid.
    double delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, _clientQueue, ^(void){
        [self _login:[notification.userInfo objectForKey:kAirFloatDAAPPairerGuidKey]];        
    });
    
}

#pragma mark - Public Properties

@synthesize delegate=_delegate;
@synthesize clientType=_clientType;

#pragma mark - Private Methods

- (NSData*)_dataForCommand:(NSString *)command statusCode:(NSInteger *)statusCode {
    
    NSString* cmd = command;
    if (_clientType == kAirFloatDAAPClientTypeDAAP && _session > 0) {
        if ([command rangeOfString:@"?"].location == NSNotFound)
            cmd = [NSString stringWithFormat:@"%@?session-id=%d", command, _session];
        else
            cmd = [command stringByReplacingOccurrencesOfString:@"?" withString:[NSString stringWithFormat:@"?session-id=%d&", _session]];
    }
    
    NSString* urlString = [NSString stringWithFormat:@"http://%@/%@", _host, cmd];
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:urlString]];
    
    if (_clientType == kAirFloatDAAPClientTypeDACP) {
        [request addValue:_activeRemove forHTTPHeaderField:@"Active-Remote"];
        [request addValue:_dacpId forHTTPHeaderField:@"DACP-ID"];
    } else
        [request addValue:@"1" forHTTPHeaderField:@"Viewer-Only-Client"];
    
    NSHTTPURLResponse* response = nil;
    NSError* error = nil;
    
    NSData* ret = [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];
    
    if (!error) {
        
        if (statusCode != NULL)
            *statusCode = [response statusCode];
        
        NSDLog(@"%d %@", [response statusCode], cmd);
        
        return ret;
        
    } else
        NSDLog(@"%@", [error description]);
        
    return nil;
    
}

- (NSData*)_dataForCommand:(NSString *)command {
    
    return [self _dataForCommand:command statusCode:NULL];
    
}

- (NSDictionary*)_executeCommand:(NSString*)command statusCode:(NSInteger*)statusCode {
    
    NSInteger myStatusCode;
    
    NSData* data = [self _dataForCommand:command statusCode:&myStatusCode];
    
    if (data && myStatusCode == 200)
        return [NSDictionary dictionaryWithDmapTaggedData:data];    
    
    return nil;
    
}

- (NSDictionary*)_executeCommand:(NSString*)command {
    
    return [self _executeCommand:command statusCode:NULL];
    
}

- (void)_playSpecific:(NSNotification *)noticiation {
    
    void (^playSpecificBlock)() = ^{
        
        NSInteger trackId = [[[noticiation userInfo] objectForKey:kAirFloatDAAPPlaySpecificIdentifierKey] integerValue];
        
        if (trackId) {
            
            NSData* databasePersistentId = [self _persistantIdForAtom:[[_nowPlaying objectForKey:kAirFloatDAAPPlaylistDatabaseIdentifierKey] integerValue]];
            NSData* playlistPersistentId = [self _persistantIdForAtom:[[_nowPlaying objectForKey:kAirFloatDAAPPlaylistIdentifierKey] integerValue]];
            
            if (databasePersistentId && playlistPersistentId)
                [self _executeCommand:[NSString stringWithFormat:@"ctrl-int/1/playspec?database-spec='dmap.persistentid:0x%@'&container-spec='dmap.persistentid:0x%@'&container-item-spec='dmap.containeritemid:0x%X'", [databasePersistentId hexEncodedString], [playlistPersistentId hexEncodedString], trackId]];
            
        }
        
    };
    
    if (dispatch_get_current_queue() != _clientQueue)
        dispatch_async(_clientQueue, playSpecificBlock);
    else
        playSpecificBlock();
    
}

- (NSData*)_persistantIdForAtom:(NSUInteger)identifier updated:(BOOL)updated {
    
    for (NSDictionary* database in _databases) {
        
        if ([[database objectForKey:kAirFloatDatabaseIdKey] integerValue] == identifier)
            return [database objectForKey:kAirFloatDatabasePersistentIdKey];
        
        for (NSDictionary* playlist in [database objectForKey:kAirFloatDatabasePlaylistsKey])
            if ([[playlist objectForKey:kAirFloatPlaylistIdKey] integerValue] == identifier)
                return [playlist objectForKey:kAirFloatPlaylistPersistentIdKey];
        
    }
    
    if (!updated) {
        [self _updateDatabases];
        return [self _persistantIdForAtom:identifier updated:YES];
    }
        
    return nil;
    
}

- (NSData*)_persistantIdForAtom:(NSUInteger)identifier {
    
    return [self _persistantIdForAtom:identifier updated:NO];
    
}

- (void)_updateDatabases {
    
    void (^updateBlock)() = ^{
        
        NSDictionary* serverDatabases = [[self _executeCommand:@"databases?meta=dmap.itemid,dmap.persistentid"] objectForKey:@"daap.serverdatabases"];
        
        if (serverDatabases) {
            
            NSMutableArray* databases = [[NSMutableArray alloc] init];
            
            for (NSDictionary* serverDatabase in [serverDatabases objectForKey:@"dmap.listing"]) {
                
                NSDictionary* serverPlaylists = [[self _executeCommand:[NSString stringWithFormat:@"databases/%d/containers?meta=dmap.itemid,dmap.persistentid", [[serverDatabase objectForKey:@"dmap.itemid"] integerValue]]] objectForKey:@"daap.databaseplaylists"];
                
                NSMutableArray* playlists = [[NSMutableArray alloc] init];
                
                for (NSDictionary* serverPlaylist in [serverPlaylists objectForKey:@"dmap.listing"])                    
                    [playlists addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                                          [serverPlaylist objectForKey:@"dmap.itemid"], kAirFloatPlaylistIdKey,
                                          [serverPlaylist objectForKey:@"dmap.persistentid"], kAirFloatPlaylistPersistentIdKey,
                                          nil]];
                
                [databases addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                                      [serverDatabase objectForKey:@"dmap.itemid"], kAirFloatDatabaseIdKey,
                                      [serverDatabase objectForKey:@"dmap.persistentid"], kAirFloatDatabasePersistentIdKey,
                                      playlists, kAirFloatDatabasePlaylistsKey,
                                      nil]];
                [playlists release];
                
            }
            
            [_databases release];
            _databases = [databases copy];
            
            [databases release];
                        
        }
                
    };
    
    if (dispatch_get_current_queue() != _clientQueue)
        dispatch_async(_clientQueue, updateBlock);
    else
        updateBlock();
    
}

- (void)_login:(NSData *)guid {
    
    void(^loginBlock)() = ^{
        
        _clientType = kAirFloatDAAPClientTypeDAAP;
        
        NSDictionary* loginResponse = [[self _executeCommand:[NSString stringWithFormat:@"login?pairing-guid=0x%@", [guid hexEncodedString]]] objectForKey:@"dmap.loginresponse"];
        
        if (loginResponse) {
            
            _session = [[loginResponse objectForKey:@"dmap.sessionid"] integerValue];
            
            if (_session > 0)
                NSDLog(@"DACP Client registered as DAAP (Session: %d)", _session);
            
        } else
            [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPClientFailedAuthenticationNotification
                                                       object:self
                                                     userInfo:[NSDictionary dictionaryWithObject:guid 
                                                                                          forKey:kAirFloatDAAPClientGuidKey]];
        
        if (![self _updateServerInfo]) {
            
            if (_clientType == kAirFloatDAAPClientTypeDAAP) {
                _clientType = kAirFloatDAAPClientTypeDACP;
                if (![self _updateServerInfo])
                    [self.delegate loginFailedInDaapClientAndCannotFallbackToDacp:self];
            }
            
        }

    };
    
    if (dispatch_get_current_queue() != _clientQueue)
        dispatch_async(_clientQueue, loginBlock);
    else
        loginBlock();
    
}

- (void)_delayedUpdatePlaybackStatus {
    
    double delayInSeconds = 2.5;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, delayInSeconds * NSEC_PER_SEC);
    dispatch_after(popTime, _clientQueue, ^(void){
        [self _updatePlaybackStatus];
    });
    
}

- (BOOL)_updatePlaybackStatus {
    
    BOOL (^updatePlaybackStatusBlock)() = ^{
        NSDictionary* playStatus = [[self _executeCommand:@"ctrl-int/1/playstatusupdate"] objectForKey:@"com.AirFloat.NowPlayingContainer"];
        
        if (playStatus) {
            
            NSNumber* playbackStatus = [NSNumber numberWithInteger:[[playStatus objectForKey:@"com.AirFloat.NowPlayingStatus"] integerValue]];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackStatusUpdatedNotification object:self userInfo:[NSDictionary dictionaryWithObject:playbackStatus forKey:kAirFloatDAAPPlaybackStatusKey]];
            });
            
            if (_clientType == kAirFloatDAAPClientTypeDAAP) {
                
                NSData* nowPlayingInfo = [playStatus objectForKey:@"com.AirFloat.NowPlayingInfo"];
                if (nowPlayingInfo) {
                    
                    AirFloatDAAPNowPlayingInfo info = *((AirFloatDAAPNowPlayingInfo*)[nowPlayingInfo bytes]);
                    info.databaseId = ntohl(info.databaseId);
                    info.containerId = ntohl(info.containerId);
                    info._unused = ntohl(info._unused);
                    info.itemId = ntohl(info.itemId);
                    
                    if ([[_nowPlaying objectForKey:kAirFloatDAAPPlaylistIdentifierKey] integerValue] != info.containerId) {
                        
                        NSDictionary* playlist = [[self _executeCommand:[NSString stringWithFormat:@"databases/%d/containers/%d/items?meta=dmap.itemname,dmap.itemid,dmap.containeritemid,daap.songartist,daap.songalbum,daap.songtime&type=music&query='*'", info.databaseId, info.containerId]] objectForKey:@"daap.playlistsongs"];
                        
                        NSMutableArray* nowPlayingItems = [[NSMutableArray alloc] init];
                        NSUInteger nowPlayingIndex = NSNotFound;
                        
                        for (NSUInteger i = 0 ; i < [[playlist objectForKey:@"dmap.listing"] count] ; i++) {
                            
                            NSDictionary* song = [[playlist objectForKey:@"dmap.listing"] objectAtIndex:i];
                            
                            NSMutableDictionary* newItem = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                                            [song objectForKey:@"dmap.containeritemid"], kAirFloatDAAPItemIdenfifierKey,
                                                            [song objectForKey:@"dmap.itemid"], kAirFloatDAAPItemOwnIdKey,
                                                            [NSNull nullIfNil:[song objectForKey:@"dmap.itemname"]], kAirFloatDAAPItemNameKey,
                                                            [NSNull nullIfNil:[song objectForKey:@"daap.songartist"]], kAirFloatDAAPItemArtistNameKey,
                                                            [NSNull nullIfNil:[song objectForKey:@"daap.songalbum"]], kAirFloatDAAPItemAlbumNameKey,
                                                            [song objectForKey:@"daap.songtime"], kAirFloatDAAPItemDurationKey,
                                                            nil];
                            
                            if ([[song objectForKey:@"dmap.itemid"] integerValue] == info.itemId)
                                nowPlayingIndex = i;
                            
                            [nowPlayingItems addObject:newItem];
                            
                        }
                        
                        NSMutableDictionary* nowPlaying = [[NSMutableDictionary alloc] initWithObjectsAndKeys:
                                                           [NSNumber numberWithInteger:info.databaseId], kAirFloatDAAPPlaylistDatabaseIdentifierKey,
                                                           [NSNumber numberWithInteger:info.containerId], kAirFloatDAAPPlaylistIdentifierKey,
                                                           nowPlayingItems, kAirFloatDAAPPlaylistItemsKey,
                                                           nil];
                        
                        if (nowPlayingIndex != NSNotFound)
                            [nowPlaying setObject:[NSNumber numberWithInteger:nowPlayingIndex] forKey:kAirFloatDAAPPlaylistItemsPlayingSongIndexKey];
                        
                        [nowPlayingItems release];
                        
                        [_nowPlaying release];
                        _nowPlaying = nowPlaying;
                        
                    } else {
                        
                        // Play status was updated, but playing playlist didn't change. Find and set the current playing track.
                        
                        NSArray* nowPlayingItems = [_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsKey];
                        for (NSInteger i = 0 ; i < [nowPlayingItems count] ; i++)
                            if ([[[nowPlayingItems objectAtIndex:i] objectForKey:kAirFloatDAAPItemOwnIdKey] integerValue] == info.itemId) {
                                [_nowPlaying setObject:[NSNumber numberWithInteger:i] forKey:kAirFloatDAAPPlaylistItemsPlayingSongIndexKey];
                                break;
                            }
                        
                    }
                    
                    dispatch_async(dispatch_get_main_queue(), ^{
                        [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPClientDidUpdatePlaylistNotification object:self userInfo:_nowPlaying];
                    });
                    
                }
                
            }
            
            return YES;
                        
        } else
            dispatch_async(dispatch_get_main_queue(), ^{
                [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPClientIsControllableNotification object:nil];
            });
        
        return NO;
        
    };
    
    if (dispatch_get_current_queue() != _clientQueue) {
        dispatch_async(_clientQueue, ^{
            updatePlaybackStatusBlock();
        });
        return YES;
    } else
        return updatePlaybackStatusBlock();
    
}

- (void)_updateServerInfo {
    
    if (dispatch_get_current_queue() != _clientQueue)
        dispatch_async(_clientQueue, ^{
            
            [self _executeCommand:@"server-info"];
            [self _updatePlaybackStatus];
            
        });
    else {
        [self _executeCommand:@"server-info"];
        [self _updatePlaybackStatus];
    }
    
}

- (void)_playPause {
    
    if (dispatch_get_current_queue() != _clientQueue)
        dispatch_async(_clientQueue, ^{
            [self _executeCommand:@"ctrl-int/1/playpause"];
        });
    else
        [self _executeCommand:@"ctrl-int/1/playpause"];
    
}

- (void)_next {
    
    if (dispatch_get_current_queue() != _clientQueue)
        dispatch_async(_clientQueue, ^{
            [self _executeCommand:@"ctrl-int/1/nextitem"];
        });
    else
        [self _executeCommand:@"ctrl-int/1/nextitem"];
    
}

- (void)_prev {
    
    if (dispatch_get_current_queue() != _clientQueue)
        dispatch_async(_clientQueue, ^{
            [self _executeCommand:@"ctrl-int/1/previtem"];
        });
    else
        [self _executeCommand:@"ctrl-int/1/previtem"];
    
}

@end

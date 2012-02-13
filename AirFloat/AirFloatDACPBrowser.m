//
//  AirFloatDACPBrowser.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/12/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatDACPBrowser.h"

@interface AirFloatDACPBrowser (Private)

- (void)_searchNextDomain;
- (void)_resolveNextService;

@end

@implementation AirFloatDACPBrowser

@synthesize delegate=_delegate;

- (id)init {
    
    if ((self = [super init])) {
        
        _netServiceBrowser = [[NSNetServiceBrowser alloc] init];
        [_netServiceBrowser setDelegate:self];
        
        _foundDomains = [[NSMutableArray alloc] init];
        _foundServices = [[NSMutableArray alloc] init];
        
    }
    
    return self;
    
}

- (void)dealloc {
    
    [_netServiceBrowser stop];
    [_netServiceBrowser release];
    
    [_foundDomains release];
    [_foundServices release];
    
    [_dacpId release];
    
    [super dealloc];
    
}

- (void)findServerForIdentifier:(NSString *)dacpId {
    
    [_dacpId release];
    _dacpId = [dacpId retain];
    
    [_netServiceBrowser stop];
    [_netServiceBrowser searchForBrowsableDomains];
    
}

#pragma mark Private Methods

- (void)_searchNextDomain {
    
    if ([_foundDomains count] > 0) {
        
        [_netServiceBrowser stop];
        
        NSString* domain = [_foundDomains objectAtIndex:0];
        [_netServiceBrowser searchForServicesOfType:@"_dacp._tcp." inDomain:domain];
        [_foundDomains removeObjectAtIndex:0];

    }
    
}

- (void)_resolveNextService {
    
    if ([_foundServices count] > 0) {
        
        NSNetService* service = [_foundServices objectAtIndex:0];
        [service resolveWithTimeout:5.0];
        
    } else 
        [self _searchNextDomain];
    
}

#pragma mark NSNetServiceBrowser Delegate Methods

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindDomain:(NSString *)domainString moreComing:(BOOL)moreComing {
    
    [_foundDomains addObject:domainString];
    
    if (!moreComing)
        [self _searchNextDomain];
    
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
    
    [aNetService setDelegate:self];
    [_foundServices addObject:aNetService];
    
    if (!moreComing)
        [self _resolveNextService];
    
}

#pragma mark NSNetService Delegate Methods

- (void)netServiceDidResolveAddress:(NSNetService *)sender {
    
    if ([[sender name] isEqualToString:[NSString stringWithFormat:@"iTunes_Ctrl_%@", _dacpId]]) {
        
        if (self.delegate)
            [self.delegate dacpBrowser:self didFindHost:[NSString stringWithFormat:@"%@:%d", [sender hostName], [sender port]]];
        
        [_foundServices removeAllObjects];
        [_foundDomains removeAllObjects];
        
    } else {
    
        [_foundServices removeObject:sender];    
        [self _resolveNextService];
        
    }
    
}

- (void)netService:(NSNetService *)sender didNotResolve:(NSDictionary *)errorDict {
    
    [_foundServices removeObject:sender];    
    [self _resolveNextService];
    
}

@end

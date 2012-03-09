//
//  AirFloatBonjourBrowser.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/12/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <sys/socket.h>
#import <arpa/inet.h>
#import "AirFloatAdditions.h"
#import "AirFloatBonjourBrowser.h"

@interface AirFloatBonjourBrowser (Private)

- (void)_searchTimedOut;
- (void)_searchNextDomain;
- (void)_resolveNextService;

@end

@implementation AirFloatBonjourBrowser

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
    
    [_serviceType release];
    
    [_netServiceBrowser stop];
    [_netServiceBrowser release];
    
    [_foundDomains release];
    [_foundServices release];
    
    [super dealloc];
    
}

- (void)findService:(NSString *)service {
    
    [_netServiceBrowser stop];
    
    NSDLog(@"findService:@\"%@\"", service);

    [_serviceType release];
    _serviceType = [service retain];
    
    [self performSelector:@selector(_searchTimedOut) withObject:nil afterDelay:5.0];
    
    [_netServiceBrowser searchForBrowsableDomains];
    
}

#pragma mark - Private Methods

- (void)_searchTimedOut {
    
    [self.delegate bonjourBrowser:self endedSearchForServiceType:_serviceType];
    
}

- (void)_searchNextDomain {
    
    if ([_foundDomains count] > 0) {
        
        [_netServiceBrowser stop];
        
        NSString* domain = [_foundDomains objectAtIndex:0];
        [_netServiceBrowser searchForServicesOfType:_serviceType inDomain:domain];
        [_foundDomains removeObjectAtIndex:0];

    } else
        [self.delegate bonjourBrowser:self endedSearchForServiceType:_serviceType];
    
}

- (void)_resolveNextService {
    
    if ([_foundServices count] > 0) {
        
        NSNetService* service = [_foundServices objectAtIndex:0];
        [service resolveWithTimeout:5.0];
        
    } else 
        [self _searchNextDomain];
    
}

#pragma mark - NSNetServiceBrowser Delegate Methods

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindDomain:(NSString *)domainString moreComing:(BOOL)moreComing {
    
    [_foundDomains addObject:domainString];
    
    if (!moreComing)
        [self _searchNextDomain];
    
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)aNetServiceBrowser didFindService:(NSNetService *)aNetService moreComing:(BOOL)moreComing {
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    [aNetService setDelegate:self];
    [_foundServices addObject:aNetService];
    
    if (!moreComing)
        [self _resolveNextService];
    
}

#pragma mark - NSNetService Delegate Methods

- (void)netServiceDidResolveAddress:(NSNetService *)sender {
    
    if (![self.delegate bonjourBrowser:self didFindAddresses:sender.addresses forService:sender]) {
        
        [_foundServices removeObject:sender];    
        [self _resolveNextService];
        
    } else {
        
        [_foundServices removeAllObjects];
        [_foundDomains removeAllObjects];
        
    }
        
}

- (void)netService:(NSNetService *)sender didNotResolve:(NSDictionary *)errorDict {
    
    [_foundServices removeObject:sender];    
    [self _resolveNextService];
    
}

@end

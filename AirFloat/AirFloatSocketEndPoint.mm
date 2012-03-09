//
//  AirFloatSocketEndPoint.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <netinet/in.h>
#import "AirFloatSocketEndPoint+Private.h"

@implementation AirFloatSocketEndPoint

#pragma - Allocation / Deallocation

- (id)_initWithEndPoint:(SocketEndPoint *)endPoint {
    
    if ((self = [super init]))
        _endPoint = endPoint->copy();
    
    return self;
    
}

- (void)dealloc {
    
    delete (SocketEndPoint*)self._endPoint;
    
    [super dealloc];
    
}

#pragma mark - Public Methods

- (BOOL)isEqualToAddress:(struct sockaddr *)address {
    
    return self._endPoint->compareWithAddress(address);
    
}

#pragma mark - Public Properties

- (NSString*)host {
    
    char host[INET6_ADDRSTRLEN];
    self._endPoint->getHost(host, INET6_ADDRSTRLEN);
    return [NSString stringWithCString:host encoding:NSASCIIStringEncoding];
    
}

- (NSUInteger)port {
    
    return self._endPoint->getPort();
    
}

- (sockaddr*)address {
    
    return self._endPoint->getSocketAddress();
    
}

- (BOOL)isIPv6 {
    
    return self._endPoint->isIPv6();
    
}

#pragma mark - Private Properties

- (SocketEndPoint*)_endPoint {
    
    return (SocketEndPoint*)_endPoint;
    
}

@end

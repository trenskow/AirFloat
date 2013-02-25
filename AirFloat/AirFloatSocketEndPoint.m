//
//  AirFloatSocketEndPoint.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <netinet/in.h>
#import "sockaddr.h"

#import "AirFloatSocketEndPoint+Private.h"

@implementation AirFloatSocketEndPoint

#pragma - Allocation / Deallocation

- (id)_initWithEndPoint:(struct sockaddr*)endPoint {
    
    if ((self = [super init]))
        _endPoint = sockaddr_copy(endPoint);
    
    return self;
    
}

- (void)dealloc {
    
    sockaddr_destroy(self._endPoint);
    
    [super dealloc];
    
}

#pragma mark - Public Methods

- (BOOL)isEqualToAddress:(struct sockaddr *)address {
    
    return sockaddr_equals_host(self._endPoint, address);
    
}

#pragma mark - Public Properties

- (NSString*)host {
    
    const char* host = sockaddr_get_host(self._endPoint);
    return [NSString stringWithCString:host encoding:NSASCIIStringEncoding];
    
}

- (NSUInteger)port {
    
    return sockaddr_get_port(self._endPoint);
    
}

- (struct sockaddr*)address {
    
    return self._endPoint;
    
}

- (BOOL)isIPv6 {
    
    return sockaddr_is_ipv6(self._endPoint);
    
}

#pragma mark - Private Properties

- (struct sockaddr *)_endPoint {
    
    return _endPoint;
    
}

@end

//
//  AirFloatWebResponse.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "WebHeaders.h"
#import "AirFloatWebResponse+Private.h"

@implementation AirFloatWebResponse

#pragma mark Allocaion / Deallocation

- (id)_initWithResponse:(WebResponse *)response {
    
    if ((self = [super init])) {
        _response = response;
        
        WebHeaders* headers = self._response->getHeaders();
        
        NSUInteger headerCount = headers->getCount();
        _headers = [[NSMutableDictionary alloc] initWithCapacity:headerCount];
        
        for (NSUInteger i = 0 ; i < headerCount ; i++) {
            
            const char* key = headers->getNameAtIndex(i);
            [_headers setObject:[NSString stringWithCString:headers->valueForName(key) encoding:NSASCIIStringEncoding]
                         forKey:[NSString stringWithCString:key encoding:NSASCIIStringEncoding]];
            
        }
        
    }
    
    return self;
    
}

- (void)dealloc {
    
    [_headers release];
    
    [super dealloc];
    
}

#pragma makr - Private Propertes

- (WebResponse*)_response {
    
    return (WebResponse*)_response;
    
}

#pragma mark - Public Properties

- (NSDictionary*)headers {
    
    return [[_headers copy] autorelease];
    
}

#pragma mark - Public Methods

- (void)addHeaderValue:(NSString *)value forKey:(NSString *)key {
    
    [_headers setObject:value forKey:key];
    self._response->getHeaders()->addValue([key cStringUsingEncoding:NSASCIIStringEncoding], [value cStringUsingEncoding:NSASCIIStringEncoding]);
    
}

- (void)setStatusMessage:(NSString *)statusMessage forCode:(NSUInteger)code {
    
    self._response->setStatus(code, [statusMessage cStringUsingEncoding:NSASCIIStringEncoding]);
    
}

- (void)setBody:(NSData *)body {
    
    self._response->setContent((char*)[body bytes], [body length]);
    
}

- (void)setKeepAlive:(BOOL)keepAlive {
    
    self._response->setKeepAlive(keepAlive);
    
}

@end

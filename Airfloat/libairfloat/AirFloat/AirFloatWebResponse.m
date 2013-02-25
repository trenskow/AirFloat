//
//  AirFloatWebResponse.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatWebResponse+Private.h"

@implementation AirFloatWebResponse

#pragma mark Allocaion / Deallocation

- (id)_initWithResponse:(web_response_p)response {
    
    if ((self = [super init])) {
        _response = response;
        
        web_headers_p headers = web_response_get_headers(self._response);
        
        NSUInteger headerCount = web_headers_count(headers);
        _headers = [[NSMutableDictionary alloc] initWithCapacity:headerCount];
        
        for (NSUInteger i = 0 ; i < headerCount ; i++) {
            
            const char* key = web_headers_name(headers, i);
            [_headers setObject:[NSString stringWithCString:web_headers_value(headers, key)encoding:NSASCIIStringEncoding]
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

- (web_response_p)_response {
    
    return (web_response_p)_response;
    
}

#pragma mark - Public Properties

- (NSDictionary*)headers {
    
    return [[_headers copy] autorelease];
    
}

#pragma mark - Public Methods

- (void)addHeaderValue:(NSString *)value forKey:(NSString *)key {
    
    [_headers setObject:value forKey:key];
    web_headers_set_value(web_response_get_headers(self._response), [key cStringUsingEncoding:NSASCIIStringEncoding], [value cStringUsingEncoding:NSASCIIStringEncoding]);
    
}

- (void)setStatusMessage:(NSString *)statusMessage forCode:(NSUInteger)code {
    
    web_response_set_status(self._response, code, [statusMessage cStringUsingEncoding:NSASCIIStringEncoding]);
    
}

- (void)setBody:(NSData *)body {
    
    web_response_set_content(self._response, (unsigned char*)[body bytes], [body length]);
    
}

- (void)setKeepAlive:(BOOL)keepAlive {
    
    web_response_set_keep_alive(self._response, keepAlive);
    
}

@end

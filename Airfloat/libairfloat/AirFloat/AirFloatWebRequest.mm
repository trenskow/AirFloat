//
//  AirFloatWebRequest.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatWebConnection.h"
#import "AirFloatWebResponse+Private.h"
#import "AirFloatWebRequest+Private.h"

@implementation AirFloatWebRequest

@synthesize response=_response;

#pragma mark - Allocation / Deallocation

- (id)_initWithWebRequest:(WebRequest*)request forConnection:(AirFloatWebConnection *)connection {
    
    if ((self = [super init])) {
        
        _request = request;
        _response = [[AirFloatWebResponse alloc] _initWithResponse:request->getResponse()];
        _connection = connection;
        
    }
    
    return self;
    
}

- (void)dealloc {
    
    [_url release];
    [_response release];
    [_headers release];
    
    [super dealloc];
    
}

#pragma mark - Public Properties

- (NSString*)command {
    
    return [NSString stringWithCString:self._request->getCommand() encoding:NSASCIIStringEncoding];
    
}

- (NSURL*)url {
    
    if (!_url) {
        
        NSString* host = [NSString stringWithFormat:@"%@:%d", _connection.remoteEndPoint.host, _connection.remoteEndPoint.port];
        const char* requestedHost = requestedHost = self._request->getHeaders()->valueForName("Host");
        if (requestedHost != NULL)
            host = [NSString stringWithCString:requestedHost encoding:NSASCIIStringEncoding];
        
        _url = [[NSURL alloc] initWithString:[NSString stringWithFormat:@"%@://%@%@", 
                                              [[[[NSString stringWithCString:self._request->getProtocol() encoding:NSASCIIStringEncoding] componentsSeparatedByString:@"/"] objectAtIndex:0] lowercaseString],
                                              host,
                                              [NSString stringWithCString:self._request->getPath() encoding:NSASCIIStringEncoding]]];
    }
    
    return _url;
    
}

- (NSData*)body {
    
    NSUInteger contentLength;
    if ((contentLength = self._request->getContentLength()) > 0)        
        return [NSData dataWithBytes:self._request->getContent() length:contentLength];
    
    return nil;
    
}

- (NSDictionary*)headers {
    
    if (!_headers) {
        
        WebHeaders* webHeaders = self._request->getHeaders();
        NSUInteger count = webHeaders->getCount();
        NSMutableDictionary* headers = [[NSMutableDictionary alloc] initWithCapacity:count];
        
        for (NSUInteger i = 0 ; i < count ; i++) {
            const char* key = webHeaders->getNameAtIndex(i);
            [headers setObject:[NSString stringWithCString:webHeaders->valueForName(key) encoding:NSASCIIStringEncoding]
                        forKey:[NSString stringWithCString:key encoding:NSASCIIStringEncoding]];
        }
        
        _headers = [headers copy];
        [headers release];
        
    }
    
    return _headers;
    
}

#pragma mark Private Properties

- (WebRequest*)_request {
    
    return (WebRequest*)_request;
    
}

@end

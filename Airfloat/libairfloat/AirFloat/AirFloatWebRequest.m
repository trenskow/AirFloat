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

- (id)_initWithWebRequest:(web_request_p)request forConnection:(AirFloatWebConnection *)connection {
    
    if ((self = [super init])) {
        
        _request = request;
        _response = [[AirFloatWebResponse alloc] _initWithResponse:web_request_get_response(request)];
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
    
    return [NSString stringWithCString:web_request_get_command(self._request) encoding:NSASCIIStringEncoding];
    
}

- (NSURL*)url {
    
    if (!_url) {
        
        NSString* host = [NSString stringWithFormat:@"%@:%d", _connection.remoteEndPoint.host, _connection.remoteEndPoint.port];
        const char* requestedHost = requestedHost =  web_headers_value(web_request_get_headers(self._request), "Host");
        if (requestedHost != NULL)
            host = [NSString stringWithCString:requestedHost encoding:NSASCIIStringEncoding];
        
        _url = [[NSURL alloc] initWithString:[NSString stringWithFormat:@"%@://%@%@", 
                                              [[[[NSString stringWithCString:web_request_get_path(self._request) encoding:NSASCIIStringEncoding] componentsSeparatedByString:@"/"] objectAtIndex:0] lowercaseString],
                                              host,
                                              [NSString stringWithCString:web_request_get_path(self._request) encoding:NSASCIIStringEncoding]]];
    }
    
    return _url;
    
}

- (NSData*)body {
    
    NSUInteger contentLength;
    if ((contentLength = web_request_get_content(self._request, NULL, 0)) > 0) {
        char body[contentLength];
        web_request_get_content(self._request, body, contentLength);
        return [NSData dataWithBytes:body length:contentLength];
    }
    
    return nil;
    
}

- (NSDictionary*)headers {
    
    if (!_headers) {
        
        web_headers_p webHeaders = web_request_get_headers(self._request);
        NSUInteger count = web_headers_count(webHeaders);
        NSMutableDictionary* headers = [[NSMutableDictionary alloc] initWithCapacity:count];
        
        for (NSUInteger i = 0 ; i < count ; i++) {
            const char* key = web_headers_name(webHeaders, i);
            [headers setObject:[NSString stringWithCString:web_headers_value(webHeaders, key) encoding:NSASCIIStringEncoding]
                        forKey:[NSString stringWithCString:key encoding:NSASCIIStringEncoding]];
        }
        
        _headers = [headers copy];
        [headers release];
        
    }
    
    return _headers;
    
}

#pragma mark Private Properties

- (web_request_p)_request {
    
    return (web_request_p)_request;
    
}

@end

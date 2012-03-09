//
//  AirFloatWebResponse.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AirFloatWebResponse : NSObject {
    
    void* _response;
    NSMutableDictionary* _headers;
    
}

@property (nonatomic,readonly) NSDictionary* headers;

- (void)addHeaderValue:(NSString*)value forKey:(NSString*)key;

- (void)setStatusMessage:(NSString*)statusMessage forCode:(NSUInteger)code;
- (void)setBody:(NSData*)body;
- (void)setKeepAlive:(BOOL)keepAlive;

@end

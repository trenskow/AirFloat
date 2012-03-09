//
//  AirFloatWebRequest.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AirFloatWebResponse.h"

@class AirFloatWebConnection;

@interface AirFloatWebRequest : NSObject {
    
    void* _request;
    AirFloatWebResponse* _response;
    
    AirFloatWebConnection* _connection;
    
    NSURL* _url;
    NSDictionary* _headers;
    
}

@property (nonatomic,readonly) NSString* command;
@property (nonatomic,readonly) NSURL* url;
@property (nonatomic,readonly) NSData* body;
@property (nonatomic,readonly) AirFloatWebResponse* response;
@property (nonatomic,readonly) NSDictionary* headers;

@end

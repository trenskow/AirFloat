//
//  AirFloatWebRequest+Private.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "webrequest.h"

#import "AirFloatWebRequest.h"

@class AirFloatWebConnection;

@interface AirFloatWebRequest (Private)

@property (nonatomic,readonly) web_request_p _request;

- (id)_initWithWebRequest:(web_request_p)request forConnection:(AirFloatWebConnection*)connection;

@end
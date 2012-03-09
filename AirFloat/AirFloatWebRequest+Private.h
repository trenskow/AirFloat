//
//  AirFloatWebRequest+Private.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "WebRequest.h"
#import "AirFloatWebRequest.h"

@class AirFloatWebConnection;

@interface AirFloatWebRequest (Private)

@property (nonatomic,readonly) WebRequest* _request;

- (id)_initWithWebRequest:(WebRequest*)request forConnection:(AirFloatWebConnection*)connection;

@end
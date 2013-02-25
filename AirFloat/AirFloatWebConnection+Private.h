//
//  AirFloatWebConnection+Private.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include "webrequest.h"
#include "webserver.h"

#import "AirFloatWebConnection.h"

@interface AirFloatWebConnection (Private)

@property (nonatomic,readonly) web_connection_p _connection;

- (id)_initWithConnection:(web_connection_p)connection;

- (void)_processRequest:(web_request_p)request;
- (void)_connectionClosed;

@end
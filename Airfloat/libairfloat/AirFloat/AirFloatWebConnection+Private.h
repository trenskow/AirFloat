//
//  AirFloatWebConnection+Private.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <AirFloatLibrary/Library.h>
#import "AirFloatWebConnection.h"

@interface AirFloatWebConnection (Private)

@property (nonatomic,readonly) WebConnection* _connection;

- (id)_initWithConnection:(WebConnection*)connection;

- (void)_processRequest:(WebRequest*)request;
- (void)_connectionClosed;

@end
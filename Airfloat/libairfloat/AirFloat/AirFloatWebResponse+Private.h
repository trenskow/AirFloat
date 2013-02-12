//
//  AirFloatWebResponse+Private.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "Server.h"
#import "AirFloatWebResponse.h"

@interface AirFloatWebResponse (Private)

@property (nonatomic,readonly) WebResponse* _response;

- (id)_initWithResponse:(WebResponse*)response;

@end
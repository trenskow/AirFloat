//
//  AirFloatWebResponse+Private.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "webresponse.h"

#import "AirFloatWebResponse.h"

@interface AirFloatWebResponse (Private)

@property (nonatomic,readonly) web_response_p _response;

- (id)_initWithResponse:(web_response_p)response;

@end
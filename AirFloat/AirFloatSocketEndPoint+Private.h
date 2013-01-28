//
//  AirFloatSocketEndPoint+Private.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <AirFloatLibrary/Library.h>
#import "AirFloatSocketEndPoint.h"

@interface AirFloatSocketEndPoint (Private)

@property (nonatomic,readonly) SocketEndPoint* _endPoint;

- (id)_initWithEndPoint:(SocketEndPoint*)endPoint;

@end
//
//  AirFloatSocketEndPoint.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <sys/socket.h>
#import <Foundation/Foundation.h>

@interface AirFloatSocketEndPoint : NSObject {
    
    void* _endPoint;
    
}

@property (nonatomic,readonly) NSString* host;
@property (nonatomic,readonly) NSUInteger port;
@property (nonatomic,readonly) struct sockaddr* address;
@property (nonatomic,readonly) BOOL isIPv6;

- (BOOL)isEqualToAddress:(struct sockaddr*)address;

@end

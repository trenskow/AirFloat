//
//  AirFloatReachability.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/9/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <SystemConfiguration/SystemConfiguration.h>
#import <Foundation/Foundation.h>

#define AirFloatReachabilityChangedStatus @"AirFloatReachabilityChangedStatus"

@class AirFloatReachability;

@protocol AirFloatReachabilityDelegate

- (void)reachability:(AirFloatReachability*)sender didChangeStatus:(BOOL)reachable;

@end

@interface AirFloatReachability : NSObject {
    
    __weak id<AirFloatReachabilityDelegate> _delegate;
    SCNetworkReachabilityRef _reachability;
    
}

@property (nonatomic, assign) id<AirFloatReachabilityDelegate> delegate;
@property (readonly) BOOL isAvailable;

+ (AirFloatReachability*)wifiReachability;

@end

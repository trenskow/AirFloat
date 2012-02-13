//
//  AirFloatReachability.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/9/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <netinet/in.h>

#import "AirFloatReachability.h"

@interface AirFloatReachability (Private)

- (id)initWithNetworkAddress:(struct sockaddr_in*)addr;

- (void)_reachabilityChanged:(SCNetworkReachabilityFlags)flags;

@end

static void reachabilityCallback(SCNetworkReachabilityRef reachability, SCNetworkReachabilityFlags flags, void* info) {
    
    [(AirFloatReachability*)info _reachabilityChanged:flags];
    
}

@implementation AirFloatReachability

@synthesize delegate=_delegate;

- (id)initWithNetworkAddress:(const struct sockaddr_in*)addr {
    
    if ((self = [self init])) {

        _reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr*)addr);
        
        SCNetworkReachabilityContext context;
        context.copyDescription = NULL;
        context.release = NULL;
        context.retain = NULL;
        context.version = 0;
        context.info = self;
        
        SCNetworkReachabilitySetCallback(_reachability, reachabilityCallback, &context);
        SCNetworkReachabilityScheduleWithRunLoop(_reachability, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        
    }
    
    return self;
    
}

- (void)dealloc {
    
    _delegate = NULL;
    
    SCNetworkReachabilityUnscheduleFromRunLoop(_reachability, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    CFRelease(_reachability);
    
    _reachability = NULL;
    
}

+ (AirFloatReachability*)wifiReachability {
    
    struct sockaddr_in wifiAddress;
    bzero(&wifiAddress, sizeof(struct sockaddr_in));
    wifiAddress.sin_len = sizeof(struct sockaddr_in);
    wifiAddress.sin_family = AF_INET;
    wifiAddress.sin_addr.s_addr = IN_LINKLOCALNETNUM;
    
    return [[[self alloc] initWithNetworkAddress:&wifiAddress] autorelease];
    
}

- (BOOL)isAvailable {
    
    SCNetworkReachabilityFlags flags;
    SCNetworkReachabilityGetFlags(_reachability, &flags);
    
    bool isReachable = ((flags & kSCNetworkReachabilityFlagsReachable) > 0);
    bool isCellular = ((flags & kSCNetworkReachabilityFlagsIsWWAN) > 0);
    
    return (isReachable && !isCellular);
    
}

- (void)_reachabilityChanged:(SCNetworkReachabilityFlags)flags {
    
    if (self.delegate)
        [self.delegate reachability:self didChangeStatus:self.isAvailable];
    
    [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:AirFloatReachabilityChangedStatus object:self]];
    
}

@end

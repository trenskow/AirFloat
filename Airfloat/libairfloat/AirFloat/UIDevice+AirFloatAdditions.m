//
//  UIDevice+AirFloatAdditions.m
//  
//
//  Created by Kristian Trenskow on 3/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#include <sys/types.h>
#include <sys/sysctl.h>

#import "UIDevice+AirFloatAdditions.h"

@interface UIDevice (PrivateAdditions)

- (NSString*)_devicePlatformString;

@end

@implementation UIDevice (AirFloatAdditions)

#pragma mark - Private Methods

- (NSString*)_devicePlatformString {
    
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char machine[size + 1];
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    
    NSString* ret = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
    
    if ([ret length] >= 3 && [[ret substringToIndex:3] isEqualToString:@"x86"]) {
        NSArray* systemComponents = [self.systemVersion componentsSeparatedByString:@"."];
        return [NSString stringWithFormat:@"iPhoneSimulator%@,%@", [systemComponents objectAtIndex:0], ([systemComponents count] > 0 ? [systemComponents objectAtIndex:1] : @"0")];
    }
    
    return ret;
    
}

#pragma mark - Public Methods

- (NSString*)platformName {
    
    NSString* devicePlatform = [self _devicePlatformString];
    NSInteger versionLocation;
    if (devicePlatform && (versionLocation = [devicePlatform rangeOfCharacterFromSet:[NSCharacterSet decimalDigitCharacterSet]].location) != NSNotFound)
        return [devicePlatform substringToIndex:versionLocation];
    
    return nil;
    
}

- (CGFloat)platformVersion {
    
    NSString* devicePlatform = [self _devicePlatformString];
    NSInteger versionLocation;
    if (devicePlatform && (versionLocation = [devicePlatform rangeOfCharacterFromSet:[NSCharacterSet decimalDigitCharacterSet]].location) != NSNotFound)
        return [[[devicePlatform substringFromIndex:versionLocation] stringByReplacingOccurrencesOfString:@"," withString:@"."] doubleValue];
    
    return 0.0;
    
}

@end

//
//  UIApplication+AirFloatAdditions.m
//  
//
//  Created by Kristian Trenskow on 2/6/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "UIApplication+AirFloatAdditions.h"

@implementation UIApplication (AirFloatAdditions)

- (NSString*)applicationName {
    
    return [[NSBundle mainBundle].infoDictionary objectForKey:@"CFBundleDisplayName"];
    
}

@end

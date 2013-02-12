//
//  NSUserDefaults+AirFloatAdditions.m
//  
//
//  Created by Kristian Trenskow on 3/16/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "NSUserDefaults+AirFloatAdditions.h"

@implementation NSUserDefaults (AirFloatAdditions)

- (bool)boolForKey:(NSString*)key defaultValue:(BOOL)defaultValue {
    
    if ([self objectForKey:key])
        return [self boolForKey:key];
    
    return defaultValue;
    
}

@end

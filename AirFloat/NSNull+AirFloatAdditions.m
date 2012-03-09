//
//  NSNull+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "NSNull+AirFloatAdditions.h"

@implementation NSNull (AirFloatAdditions)

+ (id)nullIfNil:(id)obj {
    
    if (!obj)
        return [self null];
    
    return obj;
    
}

+ (id)ensureNonNil:(id)obj {
    
    if ([obj isKindOfClass:[NSNull class]])
        return nil;
    
    return obj;
    
}

@end

//
//  NSBundle+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "NSBundle+AirFloatAdditions.h"

@implementation NSBundle (AirFloatAdditions)

- (id)objectInNib:(NSString*)nib ofClass:(Class)cls {
    
    NSArray* objects = [self loadNibNamed:nib owner:nil options:nil];
    for (id object in objects)
        if ([NSStringFromClass([object class]) isEqualToString:NSStringFromClass(cls)])
            return object;
    
    return nil;
    
}

@end

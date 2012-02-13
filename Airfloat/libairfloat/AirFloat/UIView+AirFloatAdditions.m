//
//  UIView+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <objc/runtime.h>
#import <objc/message.h>
#import "UIView+AirFloatAdditions.h"

static BOOL __isAnimating;

@implementation UIView (AirFloatAdditions)

+ (void)airFloatBeginAnimations:(NSString *)animationID context:(void *)context {
    
    __isAnimating = YES;
    [self beginAnimations:animationID context:NULL];
    
}

+ (void)airFloatCommitAnimations {
    
    __isAnimating = NO;
    [self commitAnimations];
    
}

+ (BOOL)airFloatIsAnimating {
    
    return __isAnimating;
    
}

@end

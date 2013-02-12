//
//  CAKeyframeAnimation+AirFloatAdditions.m
//  
//
//  Created by Kristian Trenskow on 2/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "CAKeyframeAnimation+AirFloatAdditions.h"

@implementation CAKeyframeAnimation (AirFloatAdditions)

+ (CAKeyframeAnimation*)animationWithCustomCurve:(id (^)(CGFloat))curveBlock forKeyPath:(NSString *)keyPath {
    
    NSMutableArray* values = [NSMutableArray array];
    
    for (NSInteger i = 0 ; i <= 100 ; i++)
        [values addObject:curveBlock(i / 100.0)];
    
    CAKeyframeAnimation* ret = [CAKeyframeAnimation animationWithKeyPath:keyPath];
    ret.values = values;
    ret.calculationMode = kCAAnimationLinear;
    
    return ret;
    
}

@end

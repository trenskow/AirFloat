//
//  CAKeyframeAnimation+AirFloatAdditions.h
//  
//
//  Created by Kristian Trenskow on 2/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

@interface CAKeyframeAnimation (AirFloatAdditions)

+ (CAKeyframeAnimation*)animationWithCustomCurve:(id(^)(CGFloat value))curveBlock forKeyPath:(NSString*)keyPath;

@end

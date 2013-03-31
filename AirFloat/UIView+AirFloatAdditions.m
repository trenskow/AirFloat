//
//  UIView+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/23/13.
//
//

#import <QuartzCore/QuartzCore.h>

#import "UIView+AirFloatAdditions.h"

@implementation UIView (AirFloatAdditions)

- (void)shake {
    
    CAKeyframeAnimation* shakeAnimation = [CAKeyframeAnimation animationWithKeyPath:@"transform.translation.x"];
    
    shakeAnimation.values = @[@30.0f, @-20.0f, @20.0f, @-10.0f, @0.0f];
    
    shakeAnimation.duration = 0.3;
    shakeAnimation.removedOnCompletion = NO;
    [self.layer addAnimation:shakeAnimation forKey:@"bounceIn"];
    
}

@end

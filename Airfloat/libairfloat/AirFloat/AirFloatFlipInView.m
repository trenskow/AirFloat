//
//  AirFloatFlipInView.m
//  AdView
//
//  Created by Kristian Trenskow on 2/29/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "AirFloatFlipInView.h"

@implementation AirFloatFlipInView

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame]))
        [self awakeFromNib];
    
    return self;
    
}

- (void)awakeFromNib {
    
    CGRect frame = self.frame;
    self.layer.anchorPoint = CGPointMake(0, 0);
    self.frame = frame;
    
    self.layer.opacity = 0.0;
    
}

- (IBAction)flip:(id)sender {
    
    CATransform3D aTransform = CATransform3DIdentity;
    float zDistance = 1000;
    aTransform.m34 = 1.0 / -zDistance;	
    self.superview.layer.sublayerTransform = aTransform;

    CAKeyframeAnimation* rotateAnimation = [CAKeyframeAnimation animationWithKeyPath:@"transform"];
    
    rotateAnimation.values = [NSArray arrayWithObjects:
                              [NSValue valueWithCATransform3D:CATransform3DMakeRotation(M_PI_2, 1, 0, 0)], 
                              [NSValue valueWithCATransform3D:CATransform3DMakeRotation(-M_PI_2 / 2, 1, 0, 0)],
                              [NSValue valueWithCATransform3D:CATransform3DMakeRotation(M_PI_2 / 8, 1, 0, 0)],
                              [NSValue valueWithCATransform3D:CATransform3DMakeRotation(0, 1, 0, 0)],
                              nil];
    
    rotateAnimation.timingFunctions = [NSArray arrayWithObjects:
                                       [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn], 
                                       [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut],
                                       [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut],
                                       [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut],
                                       nil];
        
    CAKeyframeAnimation* opacityAnimation = [CAKeyframeAnimation animationWithKeyPath:@"opacity"];
    
    opacityAnimation.values = [NSArray arrayWithObjects:
                               [NSNumber numberWithDouble:0.0],
                               [NSNumber numberWithDouble:0.0],
                               [NSNumber numberWithDouble:1.0],
                               [NSNumber numberWithDouble:1.0],
                               nil];
    
    opacityAnimation.keyTimes = [NSArray arrayWithObjects:
                                 [NSNumber numberWithDouble:0.0],
                                 [NSNumber numberWithDouble:0.05],
                                 [NSNumber numberWithDouble:0.1],
                                 [NSNumber numberWithDouble:1.0],
                                 nil];
    
    rotateAnimation.duration = opacityAnimation.duration = 1.0;
    rotateAnimation.removedOnCompletion = opacityAnimation.removedOnCompletion = NO;
    rotateAnimation.fillMode = opacityAnimation.fillMode = kCAFillModeForwards;
    
    [self.layer addAnimation:rotateAnimation forKey:@"flipKey"];
    [self.layer addAnimation:opacityAnimation forKey:@"opacityKey"];
    
}

@end

//
//  AirFloatFlipInView.m
//  AdView
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    
    [super awakeFromNib];
    
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

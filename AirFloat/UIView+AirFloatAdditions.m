//
//  UIView+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "CAKeyframeAnimation+AirFloatAdditions.h"
#import "UIView+AirFloatAdditions.h"

@implementation UIView (AirFloatAdditions)

#pragma mark - Public Properties

- (CGFloat)x {
    
    return self.frame.origin.x;
    
}

- (void)setX:(CGFloat)x {
    
    CGRect frame = self.frame;
    frame.origin.x = x;
    self.frame = frame;
    
}

- (CGFloat)y {
    
    return self.frame.origin.y;
    
}

- (void)setY:(CGFloat)y {
    
    CGRect frame = self.frame;
    frame.origin.y = y;
    self.frame = frame;
    
}

- (CGFloat)width {
    
    return self.frame.size.width;
    
}

- (void)setWidth:(CGFloat)width {
    
    CGRect frame = self.frame;
    frame.size.width = width;
    self.frame = frame;
    
}

- (CGFloat)height {
    
    return self.frame.size.height;
    
}

- (void)setHeight:(CGFloat)height {
    
    CGRect frame = self.frame;
    frame.size.height = height;
    self.frame = frame;
    
}

#pragma mark - Class Methods

+ (UIView*)viewWithFrame:(CGRect)rect {
    
    return [[[self alloc] initWithFrame:rect] autorelease];
    
}

#pragma mark - Public Methods

- (UITapGestureRecognizer*)addTapGestureRecognizerWithTarget:(id)target action:(SEL)selector {
    
    UITapGestureRecognizer* tapGestureRecognizer = [[[UITapGestureRecognizer alloc] initWithTarget:target action:selector] autorelease];
    
    [self addGestureRecognizer:tapGestureRecognizer];
    
    return tapGestureRecognizer;
    
}

- (UISwipeGestureRecognizer*)addSwipeGestureRecognizerWithDirection:(UISwipeGestureRecognizerDirection)direction andTarget:(id)target action:(SEL)selector {
    
    UISwipeGestureRecognizer* swipeGestureRecognizer = [[[UISwipeGestureRecognizer alloc] initWithTarget:target action:selector] autorelease];
    
    swipeGestureRecognizer.direction = direction;
    [self addGestureRecognizer:swipeGestureRecognizer];
    
    return swipeGestureRecognizer;
    
}

- (void)shake {
    
    CAKeyframeAnimation* shakeAnimation = [CAKeyframeAnimation animationWithKeyPath:@"transform.translation.x"];
    
    shakeAnimation.values = [NSArray arrayWithObjects:
                             [NSNumber numberWithFloat:30],
                             [NSNumber numberWithFloat:-20],
                             [NSNumber numberWithFloat:20],
                             [NSNumber numberWithFloat:-10], 
                             [NSNumber numberWithFloat:0],
                             nil];
    
    shakeAnimation.duration = 0.3;
    shakeAnimation.removedOnCompletion = NO;
    [self.layer addAnimation:shakeAnimation forKey:@"bounceIn"];
    
}

@end

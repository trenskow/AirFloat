//
//  UIView+AirFloatAdditions.m
//  
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

- (CGFloat)centerX {
    
    return self.center.x;
    
}

- (void)setCenterX:(CGFloat)centerX {
    
    self.center = CGPointMake(centerX, self.center.y);
    
}

- (CGFloat)centerY {
    
    return self.center.y;
    
}

- (void)setCenterY:(CGFloat)centerY {
    
    self.center = CGPointMake(self.center.x, centerY);
    
}

- (CGPoint)origin {
    
    return self.frame.origin;
    
}

- (void)setOrigin:(CGPoint)origin {
    
    CGRect frame = self.frame;
    frame.origin = origin;
    self.frame = frame;
    
}

- (CGSize)size {
    
    return self.frame.size;
    
}

- (void)setSize:(CGSize)size {
    
    CGRect frame = self.frame;
    frame.size = size;
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

- (NSArray*)subviewsOfClass:(Class)cls {
    
    NSArray* ret = [NSArray array];
    
    for (UIView* subview in self.subviews) {
        if ([subview isKindOfClass:cls])
            ret = [ret arrayByAddingObject:subview];
        else
            ret = [ret arrayByAddingObjectsFromArray:[subview subviewsOfClass:cls]];
    }
    
    return ret;
    
}

- (UIImage*)imageRepresentation {
    
    UIGraphicsBeginImageContextWithOptions(self.bounds.size, NO, self.window.screen.scale);
    
    [self.layer renderInContext:UIGraphicsGetCurrentContext()];
    
    UIImage* ret = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();
    
    return ret;
    
}

@end

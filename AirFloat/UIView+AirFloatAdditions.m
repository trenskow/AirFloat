//
//  UIView+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "UIView+AirFloatAdditions.h"

@implementation UIView (AirFloatAdditions)

- (void)addTapGestureRecognizerWithTarget:(id)target action:(SEL)selector {
    
    [self addGestureRecognizer:[[[UITapGestureRecognizer alloc] initWithTarget:target action:selector] autorelease]];
    
}

- (void)addSwipeGestureRecognizerWithDirection:(UISwipeGestureRecognizerDirection)direction andTarget:(id)target action:(SEL)selector {
    
    UISwipeGestureRecognizer* swipeGestureRecognizer = [[[UISwipeGestureRecognizer alloc] initWithTarget:target action:selector] autorelease];
    swipeGestureRecognizer.direction = direction;
    [self addGestureRecognizer:swipeGestureRecognizer];
    
}

@end

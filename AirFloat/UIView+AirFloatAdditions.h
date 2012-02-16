//
//  UIView+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIView (AirFloatAdditions)

- (void)addTapGestureRecognizerWithTarget:(id)target action:(SEL)selector;
- (void)addSwipeGestureRecognizerWithDirection:(UISwipeGestureRecognizerDirection)direction andTarget:(id)target action:(SEL)selector;

@end

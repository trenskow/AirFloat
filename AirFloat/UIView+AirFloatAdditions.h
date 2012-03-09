//
//  UIView+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIView (AirFloatAdditions)

@property (nonatomic,assign) CGFloat x;
@property (nonatomic,assign) CGFloat y;
@property (nonatomic,assign) CGFloat width;
@property (nonatomic,assign) CGFloat height;

+ (UIView*)viewWithFrame:(CGRect)rect;

- (UITapGestureRecognizer*)addTapGestureRecognizerWithTarget:(id)target action:(SEL)selector;
- (UISwipeGestureRecognizer*)addSwipeGestureRecognizerWithDirection:(UISwipeGestureRecognizerDirection)direction andTarget:(id)target action:(SEL)selector;

- (void)shake;

@end

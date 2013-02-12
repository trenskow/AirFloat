//
//  UIView+AirFloatAdditions.h
//  
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

@property (nonatomic,assign) CGFloat centerX;
@property (nonatomic,assign) CGFloat centerY;

@property (nonatomic,assign) CGPoint origin;
@property (nonatomic,assign) CGSize size;

+ (UIView*)viewWithFrame:(CGRect)rect;

- (UITapGestureRecognizer*)addTapGestureRecognizerWithTarget:(id)target action:(SEL)selector;
- (UISwipeGestureRecognizer*)addSwipeGestureRecognizerWithDirection:(UISwipeGestureRecognizerDirection)direction andTarget:(id)target action:(SEL)selector;

- (void)shake;

- (NSArray*)subviewsOfClass:(Class)cls;

- (UIImage*)imageRepresentation;

@end

//
//  AirFloatGenericViewController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/13/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol AirFloatGenericViewController

@property (nonatomic,assign,setter=setAppereance:) BOOL apparent;

- (void)appear;
- (void)disappear;
- (void)setAppereance:(BOOL)apparent;
- (void)setAppereance:(BOOL)apparent animated:(BOOL)animated;

@end

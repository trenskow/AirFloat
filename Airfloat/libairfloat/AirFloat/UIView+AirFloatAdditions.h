//
//  UIView+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIView (AirFloatAdditions)

+ (void)airFloatBeginAnimations:(NSString *)animationID context:(void *)context;
+ (void)airFloatCommitAnimations;
+ (BOOL)airFloatIsAnimating;

@end

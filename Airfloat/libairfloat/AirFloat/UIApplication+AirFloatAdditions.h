//
//  UIApplication+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/6/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

#define UISharedApplication [UIApplication sharedApplication]

@interface UIApplication (AirFloatAdditions)

@property (readonly) NSString* applicationName;

@end

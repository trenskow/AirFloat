//
//  UIDevice+AirFloatAdditions.h
//  
//
//  Created by Kristian Trenskow on 2/20/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

#define UICurrentDevice [UIDevice currentDevice]

@interface UIDevice (AirFloatAdditions)

- (NSString*)platformName;
- (CGFloat)platformVersion;

@end
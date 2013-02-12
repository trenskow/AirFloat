//
//  NSBundle+AirFloatAdditions.h
//  
//
//  Created by Kristian Trenskow on 2/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

#define NSMainBundle [NSBundle mainBundle]

@interface NSBundle (AirFloatAdditions)

- (id)objectInNib:(NSString*)nib ofClass:(Class)cls;

@end

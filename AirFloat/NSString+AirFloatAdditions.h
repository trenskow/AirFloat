//
//  NSString+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (AirFloatAdditions)

- (NSString*)stringByRightPaddingToLength:(NSUInteger)newLength withString:(NSString *)padString startingAtIndex:(NSUInteger)padIndex;

@end

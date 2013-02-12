//
//  NSNull+AirFloatAdditions.h
//  
//
//  Created by Kristian Trenskow on 2/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSNull (AirFloatAdditions)

+ (id)nullIfNil:(id)obj;
+ (id)ensureNonNil:(id)obj;

@end

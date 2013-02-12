//
//  NSUserDefaults+AirFloatAdditions.h
//  
//
//  Created by Kristian Trenskow on 2/19/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

#define NSStandarUserDefaults [NSUserDefaults standardUserDefaults]

@interface NSUserDefaults (AirFloatAdditions)

- (bool)boolForKey:(NSString*)key defaultValue:(BOOL)defaultValue;

@end

//
//  AirFloatMacros.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#define kAirFloatUserDefaultsAuthenticationEnabledKey   @"kAirFloatUserDefaultsAuthenticationEnabledKey"
#define kAirFloatUserDefaultsPasswordKey                @"kAirFloatUserDefaultsPasswordKey"
#define kAirFloatUserDefaultsDoNotDimDisplayKey         @"kAirFloatUserDefaultsDoNotDimDisplayKey"

#if defined(DEBUG)
#define NSDLog(x, ...) NSLog(x, __VA_ARGS__)
#else
#define NSDLog(x, ...)
#endif

#define NSStringFromBool(x) (x ? @"YES" : @"NO")
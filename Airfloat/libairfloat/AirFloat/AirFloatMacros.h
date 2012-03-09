//
//  AirFloatMacros.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/5/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#if defined(DEBUG)
#define NSDLog(x, ...) NSLog(x, __VA_ARGS__)
#else
#define NSDLog(x, ...)
#endif

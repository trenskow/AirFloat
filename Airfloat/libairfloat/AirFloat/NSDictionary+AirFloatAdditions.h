//
//  NSDictionary+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSDictionary (AirFloatAdditions)

+ (NSDictionary*)dictionaryWithParametersFromURLQuery:(NSString*)query;
+ (NSDictionary*)dictionaryWithDmapTaggedData:(NSData*)dmap;

- (id)initWithParametersFromURLQuery:(NSString*)query;
- (id)initWithDmapTaggedData:(NSData*)data;

- (NSData*)dmapTaggedData;

@end

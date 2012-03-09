//
//  NSData+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSData (AirFloatAdditions)

+ (NSData*)dataWithRandomData:(NSUInteger)length;

- (id)initWithRandomData:(NSUInteger)length;

- (NSString*)hexEncodedString;

@end

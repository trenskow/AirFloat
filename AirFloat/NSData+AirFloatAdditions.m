//
//  NSData+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/18/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "NSData+AirFloatAdditions.h"

@implementation NSData (AirFloatAdditions)

+ (NSData*)dataWithRandomData:(NSUInteger)length {
    
    return [[[self alloc] initWithRandomData:length] autorelease];
    
}

- (id)initWithRandomData:(NSUInteger)length {
    
    char randomBytes[length];
    
    for (NSUInteger i = 0 ; i < length ; i++)
        randomBytes[i] = arc4random() % 0xFF;
    
    return [self initWithBytes:randomBytes length:length];
    
}

- (NSString*)hexEncodedString {
    
    static char hexValue[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    
    NSMutableString* ret = [NSMutableString stringWithCapacity:[self length] * 2];
    
    const char* bytes = [self bytes];
    
    for (NSUInteger i = 0 ; i < [self length] ; i++)
        [ret appendFormat:@"%c%c", hexValue[(bytes[i] >> 4 & 0xF)], hexValue[bytes[i] & 0xF]];
    
    return [[ret copy] autorelease];
    
}

@end

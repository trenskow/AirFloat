//
//  NSString+AirFloatAdditions.m
//  
//
//  Created by Kristian Trenskow on 2/22/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "NSString+AirFloatAdditions.h"

@implementation NSString (AirFloatAdditions)

- (NSString*)stringByRightPaddingToLength:(NSUInteger)newLength withString:(NSString *)padString startingAtIndex:(NSUInteger)padIndex {
    
    NSString* ret = [self stringByPaddingToLength:newLength withString:padString startingAtIndex:padIndex];
    
    if ([ret length] > [self length])
        ret = [[ret substringFromIndex:[self length]] stringByAppendingString:self];

    return ret;
    
}

- (NSString*)predicateFormattedStringForKey:(NSString *)key {
    
    NSArray* components = [[self stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] componentsSeparatedByString:@" "];
    NSMutableArray* queries = [NSMutableArray array];
    
    for (NSString* query in components)
        [queries addObject:[NSString stringWithFormat:@"(%@ like[cd] '*%@*')", key, query]];
    
    return [NSString stringWithFormat:@"(%@)", [queries componentsJoinedByString:@" OR "]];
    
}

@end

//
//  AirFloatAutoFillingLabel.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/20/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatAutoFillingLabel.h"

@interface AirFloatAutoFillingLabel (Private)

- (NSString*)_replacedStringForString:(NSString*)string;

@end

@implementation AirFloatAutoFillingLabel

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame]))
        [self awakeFromNib];
    
    return self;
    
}

- (void)awakeFromNib {
    
    super.text = [self _replacedStringForString:super.text];
    
}

- (void)setText:(NSString *)text {
    
    [super setText:[self _replacedStringForString:text]];
    
}

#pragma mark - Private Methods

- (NSString*)_replacedStringForString:(NSString *)string {
    
    NSRegularExpression* regex = [NSRegularExpression regularExpressionWithPattern:@"\\$\\(([A-Z]*?)\\)" options:0 error:nil];
    
    NSString* ret = [[string copy] autorelease];
    
    for (NSTextCheckingResult* result in [regex matchesInString:string options:0 range:NSMakeRange(0, [string length])]) {
        
        NSString* replacement = @"";
        
        if ([[string substringWithRange:[result rangeAtIndex:1]] isEqualToString:@"APPNAME"])
            replacement = UISharedApplication.applicationName;
        if ([[string substringWithRange:[result rangeAtIndex:1]] isEqualToString:@"DEVICENAME"])
            replacement = UICurrentDevice.name;
        
        ret = [ret stringByReplacingOccurrencesOfString:[string substringWithRange:[result rangeAtIndex:0]]
                                             withString:replacement];
    }
    
    return ret;
    
}

@end

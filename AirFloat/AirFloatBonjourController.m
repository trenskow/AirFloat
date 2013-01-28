//
//  AirFloatBonjourController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/9/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatBonjourController.h"

@implementation AirFloatBonjourController

- (id)initWithMacAddress:(NSString*)macAddress andPort:(NSUInteger)port {
    
    if ((self = [self init])) {
        
        _service = [[NSNetService alloc] initWithDomain:@"local." type:@"_raop._tcp." name:[NSString stringWithFormat:@"%@@AirFloat @ %@", macAddress, [UIDevice currentDevice].name] port:port];
        
        NSDictionary* txtRecord = [NSDictionary dictionaryWithObjectsAndKeys:
                                   @"1", @"txtvers",
                                   @"0,1", @"et",
                                   @"1", @"ek",
                                   @"16", @"ss",
                                   @"44100", @"sr",
                                   @"TCP,UDP", @"tp",
                                   @"0,1", @"cn",
                                   @"true", @"da",
                                   @"0x4", @"sf",
                                   @"65537", @"vn",
                                   @"0,1,2", @"md",
                                   @"104.29", @"vs",
                                   @"false", @"sv",
                                   @"false", @"sm",
                                   @"2", @"ch",
                                   @"44100", @"sr",
                                   ([NSStandarUserDefaults boolForKey:kAirFloatUserDefaultsAuthenticationEnabledKey defaultValue:NO] && [[NSStandarUserDefaults objectForKey:kAirFloatUserDefaultsPasswordKey] length] > 0 ? @"true" : @"false"), @"pw",
                                   nil];
        
        [_service setTXTRecordData:[NSNetService dataFromTXTRecordDictionary:txtRecord]];
        
        [_service publish];
        
    }
    
    return self;
    
}

- (void)dealloc {
    
    [_service stop];
    [_service release];
    
    [super dealloc];
    
}

@end

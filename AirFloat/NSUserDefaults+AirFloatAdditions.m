//
//  NSUserDefaults+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 13/11/2016.
//
//

#import "NSUserDefaults+AirFloatAdditions.h"

@implementation NSUserDefaults (AirFloatAdditions)

- (NSString *)name {
    
    return [self objectForKey:@"name"];
    
}

- (void)setName:(NSString *)name {
    
    [self setObject:name forKey:@"name"];
    
}

- (NSString *)password {
    
    return [self objectForKey:@"password"];
    
}

- (void)setPassword:(NSString *)password {
    
    [self setObject:password forKey:@"password"];
    
}

- (BOOL)authenticationEnabled {
    
    return [self boolForKey:@"authenticationEnabled"];
    
}

- (void)setAuthenticationEnabled:(BOOL)authenticationEnabled {
    
    [self setBool:authenticationEnabled forKey:@"authenticationEnabled"];
    
}

- (BOOL)keepScreenLit {
    
    return [self boolForKey:@"keepScreenLit"];
    
}

- (void)setKeepScreenLit:(BOOL)keepScreenLit {
    
    [self setBool:keepScreenLit forKey:@"keepScreenLit"];
    
}

- (BOOL)keepScreenLitOnlyWhenReceiving {
    
    return [self boolForKey:@"keepScreenLitOnlyWhenReceiving"];
    
}

- (void)setKeepScreenLitOnlyWhenReceiving:(BOOL)keepScreenLitOnlyWhenReceiving {
    
    [self setBool:keepScreenLitOnlyWhenReceiving forKey:@"keepScreenLitOnlyWhenReceiving"];
    
}

- (BOOL)keepScreenLitOnlyWhenConnectedToPower {
    
    return [self boolForKey:@"keepScreenLitOnlyWhenConnectedToPower"];
    
}

- (void)setKeepScreenLitOnlyWhenConnectedToPower:(BOOL)keepScreenLitOnlyWhenConnectedToPower {
    
    [self setBool:keepScreenLitOnlyWhenConnectedToPower forKey:@"keepScreenLitOnlyWhenConnectedToPower"];
    
}

- (BOOL)ignoreSourceVolume {
    
    return [self boolForKey:@"ignoreSourceVolume"];
    
}

- (void)setIgnoreSourceVolume:(BOOL)ignoreSourceVolume {
    
    [self setBool:ignoreSourceVolume forKey:@"ignoreSourceVolume"];
    
}

@end

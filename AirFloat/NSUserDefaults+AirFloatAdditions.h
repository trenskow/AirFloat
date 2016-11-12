//
//  NSUserDefaults+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 13/11/2016.
//
//

#import <Foundation/Foundation.h>

#define NSStandardUserDefaults [NSUserDefaults standardUserDefaults]

@interface NSUserDefaults (AirFloatAdditions)

@property (nonatomic) NSString *name;
@property (nonatomic) NSString *password;
@property (nonatomic) BOOL authenticationEnabled;
@property (nonatomic) BOOL keepScreenLit;
@property (nonatomic) BOOL keepScreenLitOnlyWhenReceiving;
@property (nonatomic) BOOL keepScreenLitOnlyWhenConnectedToPower;
@property (nonatomic) BOOL ignoreSourceVolume;

@end

//
//  AirFloatAppDelegate.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import "TestFlight.h"

#import "AirFloatAppDelegate.h"
#import "AirFloatAdditions.h"
#import "AirFloatViewController.h"
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

@implementation AirFloatAppDelegate

@synthesize window=_window;
@synthesize viewController=_viewController;
@synthesize serverController=_serverController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    
    [TestFlight takeOff:@"d98b9af2b7b55c408e39e74b43268c30_NjIxOTgyMDEyLTAyLTEzIDE2OjI5OjE5LjEwNjY0Mw"];
    
    AudioSessionInitialize(NULL, NULL, NULL, NULL);
    
    [[AVAudioSession sharedInstance] setPreferredIOBufferDuration:0.030 error:nil];

    UILocalNotification* localNotification = [launchOptions objectForKey:UIApplicationLaunchOptionsLocalNotificationKey];
    if (localNotification)
        [UISharedApplication cancelLocalNotification:localNotification];
    
    _serverController = [[AirFloatServerController alloc] init];
    [_serverController start];
    
    self.window.rootViewController = self.viewController = [[[AirFloatViewController alloc] init] autorelease];
    [self.window makeKeyAndVisible];
    return YES;
    
}

- (void)application:(UIApplication *)application didReceiveLocalNotification:(UILocalNotification *)notification {
    
    
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    
    [_serverController start];
    
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    
    if (_serverController) {
        
        _backgroundTask = [UISharedApplication beginBackgroundTaskWithExpirationHandler:^{
            [UISharedApplication endBackgroundTask:_backgroundTask];
        }];
        [_serverController stop];
        _serverController = nil;
        [UISharedApplication endBackgroundTask:_backgroundTask];
        
    }    
    
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    
    if (_serverController) {
        
        _backgroundTask = [UISharedApplication beginBackgroundTaskWithExpirationHandler:^{
            [UISharedApplication endBackgroundTask:_backgroundTask];
        }];
        [_serverController stop];
        [_serverController release];
        _serverController = nil;
        [UISharedApplication endBackgroundTask:_backgroundTask];

    }
    
}

- (void)dealloc
{
    
    [_viewController release];
    [_serverController release];
    [_window release];
    [_viewController release];
    [super dealloc];
    
}

@end

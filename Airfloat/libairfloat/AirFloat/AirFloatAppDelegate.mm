//
//  AirFloatAppDelegate.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "AirFloatAppDelegate.h"
#import "AirFloatViewController.h"
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

@implementation AirFloatAppDelegate

@synthesize window=_window;
@synthesize viewController=_viewController;
@synthesize serverController=_serverController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    
    AudioSessionInitialize(NULL, NULL, NULL, NULL);
    
    [[AVAudioSession sharedInstance] setPreferredIOBufferDuration:0.030 error:nil];

    _serverController = [[AirFloatServerController alloc] init];
    [_serverController start];
    
    self.window.rootViewController = self.viewController = [[AirFloatViewController alloc] init];
    [self.window makeKeyAndVisible];
    return YES;
    
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    
    if (!_serverController.hasClientConnected)
        [_serverController stop];
    
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    
    
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    
    
    if (!_serverController.isRunning)
        [_serverController start];
    
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    
    [_serverController release];
    
}

- (void)dealloc
{
    [_window release];
    [_viewController release];
    [super dealloc];
}

@end

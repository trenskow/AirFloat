//
//  AirFloatiOSAppDelegate.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import "AirFloatMacros.h"
#import "AirFloatiOSAppDelegate.h"
#import "AirFloatAdditions.h"
#import "AirFloatServerController.h"
#import "AirFloatMissingWifiViewController.h"
#import "AirFloatIntroViewController.h"
#import "AirFloatReceivingViewController.h"
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>

@interface AirFloatiOSAppDelegate (Private)

- (void)_cleanUp;
- (void)_updateIdleTimer;
- (void)_applicationDidChangeStatus;
- (void)_updateStatus:(BOOL)animated;

@end

@implementation AirFloatiOSAppDelegate

#pragma mark - Properties

@synthesize window=_window;
@synthesize serverController=_serverController;
@synthesize missingWifiViewController = _missingWifiViewController;
@synthesize introViewController = _introViewController;
@synthesize receivingViewController = _receivingViewController;

#pragma mark - Private Methods

- (void)_cleanUp {
    
    if (self.serverController && self.serverController.status != kAirFloatServerControllerStatusReceiving) {
        
        _backgroundTask = [UISharedApplication beginBackgroundTaskWithExpirationHandler:^{
            [UISharedApplication endBackgroundTask:_backgroundTask];
        }];
        
        [_serverController stop];
        [_serverController release];
        _serverController = nil;
        
        [UISharedApplication endBackgroundTask:_backgroundTask];
        
    }    

}

- (void)_updateIdleTimer {
    
    BOOL isEnabled = [NSStandarUserDefaults boolForKey:kAirFloatUserDefaultsDoNotDimDisplayKey defaultValue:YES];
    BOOL isCharging = (UICurrentDevice.batteryState == UIDeviceBatteryStateCharging || UICurrentDevice.batteryState == UIDeviceBatteryStateFull);
    BOOL isRunning = (self.serverController.status > kAirFloatServerControllerStatusNeedsWifi);
    BOOL isForeground = (UISharedApplication.applicationState == UIApplicationStateActive);
    
    UISharedApplication.idleTimerDisabled = (isEnabled && isCharging && isRunning && isForeground);
    
    NSDLog(@"Idle timer disabled: %@", (UISharedApplication.idleTimerDisabled ? @"YES" : @"NO"));
    
}

- (void)_updateStatus:(BOOL)animated {
    
    // We cannot determine our state, so we wait until it is updated to determine what to display
    
    [self _updateIdleTimer];
    
    if (self.serverController.status == kAirFloatServerControllerStatusUnknown)
        return;
    
    if (self.serverController.status == kAirFloatServerControllerStatusReceiving)
        [self.receivingViewController setAppereance:YES animated:animated];
    else
        [self.receivingViewController setAppereance:NO animated:animated];
    
    if (self.serverController.status == kAirFloatServerControllerStatusReady)
        [self.introViewController setAppereance:YES animated:animated];
    else
        [self.introViewController setAppereance:NO animated:animated];
    
    if (self.serverController.status == kAirFloatServerControllerStatusNeedsWifi)
        [self.missingWifiViewController setAppereance:YES animated:animated];
    else
        [self.missingWifiViewController setAppereance:NO animated:animated];
    
}

- (void)_applicationDidChangeStatus {
    
    [self _updateStatus:YES];
    
}

#pragma mark - Public Methods

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    
    AudioSessionInitialize(NULL, NULL, NULL, NULL);
    
    [[AVAudioSession sharedInstance] setPreferredIOBufferDuration:0.030 error:nil];

    [NSDefaultNotificationCenter addObserver:self selector:@selector(_applicationDidChangeStatus) name:AirFloatServerControllerDidChangeStatusNotification object:nil];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_updateIdleTimer) name:NSUserDefaultsDidChangeNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_updateIdleTimer) name:UIDeviceBatteryStateDidChangeNotification object:nil];
    
    UICurrentDevice.batteryMonitoringEnabled = YES;
    
    [self _updateIdleTimer];
    
    _serverController = [[AirFloatServerController alloc] init];
    [_serverController start];
    
    self.window.rootViewController = self.receivingViewController;
    
    [self.window addSubview:self.missingWifiViewController.view];
    [self.window addSubview:self.introViewController.view];
    
    [self.window makeKeyAndVisible];
    
    [self _updateStatus:YES];
    
    return YES;
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    
    [self _updateStatus:YES];
    
}

- (void)applicationWillResignActive:(UIApplication *)application {
    
    [self _updateIdleTimer];
    
    if (self.introViewController.apparent)
        self.introViewController.apparent = NO;
    
    [self _cleanUp];
    
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    
    if (!_serverController)
        _serverController = [[AirFloatServerController alloc] init];
    
    [_serverController start];
        
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    
    [self _cleanUp];
    
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    
    [self _cleanUp];
    
}

- (void)dealloc
{
    
    [_serverController release];
    [_window release];
    [_missingWifiViewController release];
    [_introViewController release];
    [_receivingViewController release];
    
    [super dealloc];
    
}

@end

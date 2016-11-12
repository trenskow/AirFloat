//
//  AirFloatiOSAppDelegate.m
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#import "NSUserDefaults+AirFloatAdditions.h"

#import "AppViewController.h"
#import "AirFloatAppDelegate.h"
#import <libairfloat/audiooutput.h>

@interface AirFloatAppDelegate ()

@property (nonatomic, assign) raop_server_p server;

@end
    
@implementation AirFloatAppDelegate {
    UIBackgroundTaskIdentifier *_backgroundTask;
}


#pragma mark - NSApplication delegates

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    self.appViewController = [[AppViewController alloc] init];
    
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    
    if ([self.window respondsToSelector:@selector(setRootViewController:)])
        self.window.rootViewController = self.appViewController;
    else {
        self.appViewController.view.frame = CGRectMake(0, 20, 320, 460);
        [self.window addSubview:self.appViewController.view];
    }
    
    [self.window makeKeyAndVisible];
    
    if ([UIApplication instancesRespondToSelector:@selector(registerUserNotificationSettings:)]){
        [application registerUserNotificationSettings:[UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert|UIUserNotificationTypeBadge|UIUserNotificationTypeSound categories:nil]];
    }
    
    audio_output_session_start();
    
    return YES;
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    [self startRaopServer];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    [self.appViewController handleForegroundTasks];
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    [self.appViewController handleBackgroundTasks];
}

#pragma mark - RAOP Server interface

- (void)startRaopServer  {
    
    if (!self.server) {
        struct raop_server_settings_t settings;
        settings.name =  [NSStandardUserDefaults.name UTF8String];
        settings.password = (NSStandardUserDefaults.authenticationEnabled ? [NSStandardUserDefaults.password UTF8String] : NULL);
        settings.ignore_source_volume = NSStandardUserDefaults.ignoreSourceVolume;
        self.server = raop_server_create(settings);
    }
    
    if (!raop_server_is_running(self.server)) {
        
        uint16_t port = 5000;
        while (port < 5010 && !raop_server_start(_server, port++));
        
        self.appViewController.server = _server;
        
    }
    
}

- (void)updateRaopSeverSettings {
    
    if (self.server) {
        raop_server_set_settings(self.server, (struct raop_server_settings_t) {
            [NSStandardUserDefaults.name UTF8String],
            (NSStandardUserDefaults.authenticationEnabled ? [NSStandardUserDefaults.password UTF8String] : NULL),
            NSStandardUserDefaults.ignoreSourceVolume
        });
    }
    
}

#pragma mark - Background Notifications

-(void) showNotification:(NSString*)messageTitle
{
    if (!messageTitle) {
        messageTitle = @"Stream started.";
    }
    
    UILocalNotification *notification = [[UILocalNotification alloc] init];
    notification.alertBody = messageTitle;
    notification.fireDate = [NSDate date];
    notification.soundName = UILocalNotificationDefaultSoundName;
    
    [[UIApplication sharedApplication] scheduleLocalNotification:notification];
}

@end

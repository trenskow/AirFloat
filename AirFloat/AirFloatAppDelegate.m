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

#import <libairfloat/audioqueue.h>
#import <libairfloat/raopserver.h>

#import "AppViewController.h"

#import "AirFloatAppDelegate.h"

@interface AirFloatAppDelegate ()

@property (nonatomic, assign) raop_server_p server;

@end
    
@implementation AirFloatAppDelegate {
    UIBackgroundTaskIdentifier *_backgroundTask;
    NSDictionary *_settings;
}

- (NSString *)settingsPath {

    NSString* filename = [[[NSBundle mainBundle] bundleIdentifier] stringByAppendingPathExtension:@"plist"];

//#if TARGET_IPHONE_SIMULATOR
    //NSString* path = [[NSString stringWithFormat:@"/Users/%@/Library/Preferences/", NSUserName()] stringByAppendingPathComponent:filename];
//#else
    //NSString* path = [@"/var/mobile/Library/Preferences/" stringByAppendingPathComponent:filename];
//#endif

    //return path;
    NSArray *mypaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [mypaths objectAtIndex:0];
    NSString *newpath = [documentsDirectory stringByAppendingPathComponent:filename];
    
    return newpath;
}

- (NSDictionary *)settings {
    
    if (!_settings) {
        _settings = [[NSDictionary alloc] initWithContentsOfFile:[self settingsPath]];
        _settings = (_settings ?: [[NSDictionary alloc] init]);
    }
    
    return _settings;
    
}

- (void)setSettings:(NSDictionary *)settings {
    
    [self willChangeValueForKey:@"settings"];
    
    [_settings release];
    _settings = [settings copy];
    
    [_settings writeToFile:[self settingsPath]
                atomically:YES];
    
    if (self.server) {
        
        NSString* password = [_settings objectForKey:@"password"];
        
        raop_server_set_settings(self.server, (struct raop_server_settings_t) { [[_settings objectForKey:@"name"] cStringUsingEncoding:NSASCIIStringEncoding], ([[_settings objectForKey:@"authenticationEnabled"] boolValue] && password && [password length] > 0 ? [[_settings objectForKey:@"password"] cStringUsingEncoding:NSUTF8StringEncoding] : NULL) });
        
    }
    
    [self didChangeValueForKey:@"settings"];
        
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {

    self.appViewController = [[[AppViewController alloc] init] autorelease];

    self.window = [[[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds] autorelease];
    
    if ([self.window respondsToSelector:@selector(setRootViewController:)])
        self.window.rootViewController = self.appViewController;
    else {
        self.appViewController.view.frame = CGRectMake(0, 20, 320, 460);
        [self.window addSubview:self.appViewController.view];
    }
    
    [self.window makeKeyAndVisible];
    
    return YES;
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    
    if (!self.server) {
        
        struct raop_server_settings_t settings;
        settings.name = [[self.settings objectForKey:@"name"] cStringUsingEncoding:NSUTF8StringEncoding];
        settings.password = ([[self.settings objectForKey:@"authenticationEnabled"] boolValue] ? [[self.settings objectForKey:@"password"] cStringUsingEncoding:NSUTF8StringEncoding] : NULL);
        
        self.server = raop_server_create(settings);
        
    }
    
    if (!raop_server_is_running(self.server)) {
        
        uint16_t port = 5000;
        while (port < 5010 && !raop_server_start(_server, port++));
        
        self.appViewController.server = _server;
        
    }
    
}

- (void)applicationWillResignActive:(UIApplication *)application {
    
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    
    if (self.server && !raop_server_is_recording(self.server)) {
        raop_server_stop(self.server);
        raop_server_destroy(self.server);
        self.appViewController.server = self.server = NULL;
    }
    
}

- (void)applicationWillTerminate:(UIApplication *)application {
    
}

@end

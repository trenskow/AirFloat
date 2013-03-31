//
//  AirFloatiOSAppDelegate.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import "audioqueue.h"
#import "raopserver.h"

#import "AppViewController.h"

#import "AirFloatAppDelegate.h"

@interface AirFloatAppDelegate () {
    
    NSDictionary* _settings;
    
}

@end

@implementation AirFloatAppDelegate

@synthesize window=_window;
@synthesize appViewController=_appViewController;
@synthesize server=_server;

- (NSString *)settingsPath {
    
    NSString* filename = [[[NSBundle mainBundle] bundleIdentifier] stringByAppendingPathExtension:@"plist"];
    
#if TARGET_IPHONE_SIMULATOR
    NSString* path = [[NSString stringWithFormat:@"/Users/%@/Library/Preferences/", NSUserName()] stringByAppendingPathComponent:filename];
#else
    NSString* path = [@"/var/mobile/Library/Preferences/" stringByAppendingPathComponent:filename];
#endif
    
    return path;
    
}

- (NSDictionary *)settings {
    
    if (!_settings) {
        _settings = [[NSDictionary alloc] initWithContentsOfFile:[self settingsPath]];
        _settings = (_settings ?: [[NSDictionary alloc] init]);
        NSLog(@"%@", [_settings description]);
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
        
        raop_server_set_settings(self.server, (struct raop_server_settings_t) { [[_settings objectForKey:@"name"] cStringUsingEncoding:NSUTF8StringEncoding], ([[_settings objectForKey:@"authenticationEnabled"] boolValue] && password && [password length] > 0 ? [[_settings objectForKey:@"password"] cStringUsingEncoding:NSUTF8StringEncoding] : NULL) });
        
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

//
//  AirFloatiOSAppDelegate.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppViewController.h"

#import "raopserver.h"

#define AirFloatSharedAppDelegate ((AirFloatAppDelegate*)[UIApplication sharedApplication].delegate)

@interface AirFloatAppDelegate : NSObject <UIApplicationDelegate> {
    
    raop_server_p _server;
    UIBackgroundTaskIdentifier _backgroundTask;
    
}

@property (nonatomic, strong) UIWindow *window;
@property (nonatomic, strong) AppViewController* appViewController;
@property (nonatomic, assign) raop_server_p server;
@property (nonatomic, strong) NSDictionary* settings;

@end

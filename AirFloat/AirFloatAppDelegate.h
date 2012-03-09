//
//  AirFloatAppDelegate.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AirFloatServerController.h"

#define AirFloatSharedAppDelegate ((AirFloatAppDelegate*)[UIApplication sharedApplication].delegate)

@class AirFloatViewController;

@interface AirFloatAppDelegate : NSObject <UIApplicationDelegate> {
    
    UIBackgroundTaskIdentifier _backgroundTask;
    AirFloatServerController* _serverController;
    
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@property (nonatomic, retain) IBOutlet AirFloatViewController *viewController;

@property (nonatomic ,readonly) AirFloatServerController* serverController;

@end

//
//  AirFloatiOSAppDelegate.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

#define AirFloatiOSSharedAppDelegate ((AirFloatiOSAppDelegate*)[UIApplication sharedApplication].delegate)

@class AirFloatServerController;
@class AirFloatMissingWifiViewController;
@class AirFloatIntroViewController;
@class AirFloatReceivingViewController;

@interface AirFloatiOSAppDelegate : NSObject <UIApplicationDelegate> {
    
    UIBackgroundTaskIdentifier _backgroundTask;
    AirFloatServerController* _serverController;
    
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@property (nonatomic ,readonly) AirFloatServerController* serverController;
@property (retain, nonatomic) IBOutlet AirFloatMissingWifiViewController *missingWifiViewController;
@property (retain, nonatomic) IBOutlet AirFloatIntroViewController *introViewController;
@property (retain, nonatomic) IBOutlet AirFloatReceivingViewController *receivingViewController;

@end

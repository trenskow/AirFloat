//
//  AirFloatMacAppDelegate.h
//  AirFloat
//
//  Created by Kristian Trenskow on 4/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AirFloatMacAppDelegate : NSObject <NSApplicationDelegate> {
    
    NSStatusItem* _statusItem;
    
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSMenu *menu;

@end

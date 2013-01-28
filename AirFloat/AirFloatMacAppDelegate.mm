//
//  AirFloatMacAppDelegate.m
//  AirFloat
//
//  Created by Kristian Trenskow on 4/8/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatMacAppDelegate.h"

@interface AirFloatMacAppDelegate () 

- (void)_statusItemClicked:(id)sender;

@end

@implementation AirFloatMacAppDelegate

#pragma mark - Public Properties

@synthesize window = _window;
@synthesize menu = _menu;

#pragma mark - Public Methods

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    
    _statusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:30] retain];
    
    [_statusItem setImage:[NSImage imageNamed:@"StatusItemImage.png"]];
    [_statusItem setAlternateImage:[NSImage imageNamed:@"StatusItemAlternate.png"]];
    [_statusItem setAction:@selector(_statusItemClicked:)];
    [_statusItem setTarget:self];
    [_statusItem setHighlightMode:YES];
    [_statusItem setMenu:self.menu];
    
}

#pragma mark - Actions

- (void)_statusItemClicked:(id)sender {
    
    
}

@end

//
//  AirFloatIntroViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/13/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatIntroViewController.h"

@interface AirFloatIntroViewController ()

@end

@implementation AirFloatIntroViewController

#pragma mark - Allocation / Deallocation / Load / Unload

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    self.adView.images = [NSArray arrayWithContentsOfFile:[NSMainBundle pathForResource:@"Images" ofType:@"plist"]];
    
}

- (void)viewDidUnload {
    
    self.adView = nil;
    
    [super viewDidUnload];
    
}

- (void)dealloc {
    
    self.adView = nil;
    
    [super dealloc];

}

#pragma mark - AirFloat Generic View Controller Methods

- (void)appear {
    
    [self.adView startAnimation];
    
}

- (void)disappear {
    
    [self.adView stopAnimation];
    
}

- (BOOL)apparent {
    
    return self.adView.isAnimating;
    
}

- (void)setAppereance:(BOOL)apparent {
    
    if (apparent)
        [self appear];
    else
        [self disappear];
    
}

- (void)setAppereance:(BOOL)apparent animated:(BOOL)animated {
    
    [self setAppereance:apparent];
    
}

#pragma mark - Public Properties

@synthesize adView;

@end

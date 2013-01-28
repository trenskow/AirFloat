//
//  AirFloatMissingWifiViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/13/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatMissingWifiViewController.h"

@interface AirFloatMissingWifiViewController ()

@end

@implementation AirFloatMissingWifiViewController

#pragma mark - Allocation / Deallocation / Load / Unload

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    if ([UICurrentDevice.systemVersion doubleValue] < 5)
        self.airFloatLabel.font = [UIFont fontWithName:@"HelveticaNeue" size:15];
    
    self.view.hidden = YES;
    
}

- (void)viewDidUnload {
    
    self.airFloatLabel = nil;
    
    [super viewDidUnload];
    
}

- (void)dealloc {
    
    self.airFloatLabel = nil;
    
    [super dealloc];
    
}

#pragma mark - AirFloat Generic View Controller Methods

- (void)appear {
    
    [self setAppereance:YES];
    
}

- (void)disappear {
    
    [self setAppereance:NO];
    
}

- (BOOL)apparent {
    
    return !self.view.hidden;
    
}

- (void)setAppereance:(BOOL)apparent animated:(BOOL)animated {
    
    if (apparent == self.view.hidden) {
        
        if (apparent) {
            self.view.alpha = 0.0;
            self.view.hidden = NO;
        }
        
        [UIView animateWithDuration:(animated ? 2.0 : 0.0)
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut
                         animations:^{
                             self.view.alpha = (apparent ? 1.0 : 0.0);
                         } completion:^(BOOL finished) {
                             if (!apparent) {
                                 self.view.hidden = YES;
                                 self.view.alpha = 1.0;
                             }
                         }];
        
    }
    
}

- (void)setAppereance:(BOOL)apparent {
    
    return [self setAppereance:apparent animated:YES];
    
}

#pragma mark - Public Properties

@synthesize airFloatLabel;

@end

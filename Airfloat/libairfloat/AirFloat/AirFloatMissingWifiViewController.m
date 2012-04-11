//
//  AirFloatMissingWifiViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/13/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatMissingWifiViewController.h"

@interface AirFloatMissingWifiViewController ()

@end

@implementation AirFloatMissingWifiViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

@end

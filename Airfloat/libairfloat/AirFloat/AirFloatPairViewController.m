//
//  AirFloatPairViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatPairViewController.h"

@interface AirFloatPairViewController (Private)

- (void)_clientWasRejected;

@end

@implementation AirFloatPairViewController

@synthesize passcodeLetter1;
@synthesize passcodeLetter2;
@synthesize passcodeLetter3;
@synthesize passcodeLetter4;
@synthesize passcodeView;

- (void)dealloc {
    
    [_pairer release];
    _pairer = nil;
    
    [passcodeView release];
    [passcodeLetter1 release];
    [passcodeLetter2 release];
    [passcodeLetter3 release];
    [passcodeLetter4 release];
    
    [super dealloc];
    
}

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    UILabel* letters[4] = { self.passcodeLetter1, self.passcodeLetter2, self.passcodeLetter3, self.passcodeLetter4 };
    
    char passcode[5];
    passcode[4] = '\0';
    
    for (NSUInteger i = 0 ; i < 4 ; i++) {
        passcode[i] = ((arc4random() % 9) + 1) + '0';
        letters[i].text = [NSString stringWithFormat:@"%c", passcode[i]];
    }
            
    _pairer = [[AirFloatDAAPPairer alloc] initWithPasscode:[NSString stringWithCString:passcode encoding:NSASCIIStringEncoding]];
    _pairer.delegate = self;
    
    for (UIView* view in self.view.subviews)
        if ([view isKindOfClass:[UILabel class]] && !((UILabel*)view).font)
            ((UILabel*)view).font = [UIFont fontWithName:@"HelveticaNeue" size:14];
    
}

- (void)viewDidUnload {
        
    [_pairer release];
    _pairer = nil;
    
    [self setPasscodeView:nil];
    [self setPasscodeLetter1:nil];
    [self setPasscodeLetter2:nil];
    [self setPasscodeLetter3:nil];
    [self setPasscodeLetter4:nil];
    
    [super viewDidUnload];
    
}

- (IBAction)cancelButtonTouchUpInside:(id)sender {
    
    [self dismissModalViewControllerAnimated:YES];
    
}

#pragma mark - Pairer Delegate Methods

- (void)pairerDidAuthenticate:(AirFloatDAAPPairer *)pairer {
    
    [self dismissModalViewControllerAnimated:YES];
    
}

- (void)pairerClientWasRejected:(AirFloatDAAPPairer *)pairer {
    
    [self.passcodeView shake];
    
}

@end

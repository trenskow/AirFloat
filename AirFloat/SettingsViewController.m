//
//  SettingsViewController.m
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#import "UIView+AirFloatAdditions.h"
#import "NSUserDefaults+AirFloatAdditions.h"

#import "AirFloatAppDelegate.h"
#import "AirFloatSwitch.h"
#import "SettingsViewController.h"

NSString *const SettingsUpdatedNotification = @"SettingsUpdatedNotification";

@interface SettingsViewController () <UITextFieldDelegate, UIScrollViewDelegate>

@property (nonatomic, strong) IBOutlet UITextField* nameField;
@property (nonatomic, strong) IBOutlet UIView* nameFieldHighlightedBackground;
@property (nonatomic, strong) IBOutlet UIButton* nameClearButton;
@property (nonatomic, strong) IBOutlet UITextField* authenticationField;
@property (nonatomic, strong) IBOutlet UIView* authenticationFieldHighlightedBackground;
@property (nonatomic, strong) IBOutlet UIButton* authenticationClearButton;
@property (nonatomic, strong) IBOutlet AirFloatSwitch* authenticationEnabledSwitch;
@property (nonatomic, strong) IBOutlet AirFloatSwitch* keepScreenLitSwitch;
@property (nonatomic, strong) IBOutlet UILabel* keepScreenLitOnlyWhenReceivingLabel;
@property (nonatomic, strong) IBOutlet AirFloatSwitch* keepScreenLitOnlyWhenReceivingSwitch;
@property (nonatomic, strong) IBOutlet UILabel* keepScreenLitOnlyWhenConnectedToPowerLabel;
@property (nonatomic, strong) IBOutlet AirFloatSwitch* keepScreenLitOnlyWhenConnectedToPowerSwitch;
@property (nonatomic, strong) IBOutlet AirFloatSwitch* ignoreSourceVolumeSwitch;

- (void)updateVisuals;

@end

@implementation SettingsViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    ((UIScrollView*)self.view).contentSize = CGSizeMake(320, 358);
    
    self.nameField.text = NSStandardUserDefaults.name;
    self.authenticationField.text = NSStandardUserDefaults.password;
    self.authenticationEnabledSwitch.on = NSStandardUserDefaults.authenticationEnabled;
    self.keepScreenLitSwitch.on = NSStandardUserDefaults.keepScreenLit;
    self.keepScreenLitOnlyWhenReceivingSwitch.on = NSStandardUserDefaults.keepScreenLitOnlyWhenReceiving;
    self.keepScreenLitOnlyWhenConnectedToPowerSwitch.on = NSStandardUserDefaults.keepScreenLitOnlyWhenConnectedToPower;
    self.ignoreSourceVolumeSwitch.on = NSStandardUserDefaults.ignoreSourceVolume;
    
    [self updateVisuals];
    
    [self.authenticationEnabledSwitch addTarget:self action:@selector(authenticationSwitchValueChanged:) forControlEvents:UIControlEventValueChanged];
    [self.keepScreenLitSwitch addTarget:self action:@selector(litSwitchChangedValue:) forControlEvents:UIControlEventValueChanged];
    [self.keepScreenLitOnlyWhenReceivingSwitch addTarget:self action:@selector(litSwitchChangedValue:) forControlEvents:UIControlEventValueChanged];
    [self.keepScreenLitOnlyWhenConnectedToPowerSwitch addTarget:self action:@selector(litSwitchChangedValue:) forControlEvents:UIControlEventValueChanged];
    [self.ignoreSourceVolumeSwitch addTarget:self action:@selector(ignoreSourceVolumeChangedValue:) forControlEvents:UIControlEventValueChanged];
    
}

- (void)viewWillAppear:(BOOL)animated {
    
    [super viewWillAppear:animated];
    
    [self centerContent];
    
}

- (void)viewWillDisappear:(BOOL)animated {
    
    [super viewWillDisappear:animated];
    
    [self.nameField resignFirstResponder];
    [self.authenticationField resignFirstResponder];
    
}

- (IBAction)clearButtonTouchedUpInside:(id)sender {
    
    UITextField* textField = (sender == self.nameClearButton ? self.nameField : self.authenticationField);
    
    textField.text = @"";
    [textField becomeFirstResponder];
    
}

- (void)updateVisuals {
    
    self.keepScreenLitOnlyWhenReceivingSwitch.userInteractionEnabled = self.keepScreenLitOnlyWhenConnectedToPowerSwitch.userInteractionEnabled = self.keepScreenLitSwitch.on;
    
    [UIView animateWithDuration:0.3f
                          delay:0.0f
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.keepScreenLitOnlyWhenConnectedToPowerLabel.alpha = self.keepScreenLitOnlyWhenConnectedToPowerSwitch.alpha = self.keepScreenLitOnlyWhenReceivingLabel.alpha = self.keepScreenLitOnlyWhenReceivingSwitch.alpha = (self.keepScreenLitSwitch.on ? 1.0f : 0.3f);
                     } completion:nil];
    
}

- (void)updateSettings {
    
    NSStandardUserDefaults.name = self.nameField.text;
    NSStandardUserDefaults.password = self.authenticationField.text;
    NSStandardUserDefaults.authenticationEnabled = self.authenticationEnabledSwitch.on;
    NSStandardUserDefaults.keepScreenLit = self.keepScreenLitSwitch.on;
    NSStandardUserDefaults.keepScreenLitOnlyWhenReceiving = self.keepScreenLitOnlyWhenReceivingSwitch.on;
    NSStandardUserDefaults.keepScreenLitOnlyWhenConnectedToPower = self.keepScreenLitOnlyWhenConnectedToPowerSwitch.on;
    NSStandardUserDefaults.ignoreSourceVolume = self.ignoreSourceVolumeSwitch.on;
    
    [self updateVisuals];
    
    [AirFloatSharedAppDelegate updateRaopSeverSettings];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:SettingsUpdatedNotification object:nil userInfo:nil];
    
}

- (void)setAuthenticationEnabledSwitchOff {
    
    [self.authenticationEnabledSwitch setOn:NO animated:YES];
    
    [self.authenticationField becomeFirstResponder];
    
}

- (IBAction)authenticationSwitchValueChanged:(id)sender {
    
    if ([self.authenticationField.text length] == 0 && self.authenticationEnabledSwitch.on && !self.authenticationField.isFirstResponder) {
        
        [self.authenticationField shake];
        
        [self performSelector:@selector(setAuthenticationEnabledSwitchOff) withObject:nil afterDelay:0.5];
        
    }
    
    if (![self.authenticationField isFirstResponder])
        [self updateSettings];
    
}

- (IBAction)litSwitchChangedValue:(id)sender {
    
    [self updateSettings];
    
}

- (IBAction)ignoreSourceVolumeChangedValue:(id)sender {
    
    [self updateSettings];
    
}

- (void)textFieldDidBeginEditing:(UITextField *)textField {
    
    UIButton* clearButton = (textField == self.nameField ? self.nameClearButton : self.authenticationClearButton);
    UIView* highlightedBackgroundView = (textField == self.nameField ? self.nameFieldHighlightedBackground : self.authenticationFieldHighlightedBackground);
    
    if (clearButton.hidden) {
        clearButton.alpha = 0.0;
        clearButton.hidden = NO;
    
        [UIView animateWithDuration:0.5
                              delay:0.0
                            options:UIViewAnimationOptionCurveEaseOut
                         animations:^{
                             clearButton.alpha = 1.0;
                             highlightedBackgroundView.alpha = 1.0;
                         } completion:nil];
        
    }
    
}

- (void)textFieldDidEndEditing:(UITextField *)textField {
    
    UIButton* clearButton = (textField == self.nameField ? self.nameClearButton : self.authenticationClearButton);
    UIView* highlightedBackgroundView = (textField == self.nameField ? self.nameFieldHighlightedBackground : self.authenticationFieldHighlightedBackground);
    
    [self updateSettings];
    
    [UIView animateWithDuration:0.5
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         highlightedBackgroundView.alpha = 0.0;
                         if ([textField.text length] == 0)
                             clearButton.alpha = 0.0;
                     } completion:^(BOOL finished) {
                         if ([textField.text length] == 0)
                             clearButton.hidden = YES;
                     }];
    
    if (textField == self.authenticationField && [self.authenticationField.text length] == 0)
        [self.authenticationEnabledSwitch setOn:NO animated:YES];
    
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    
    [textField resignFirstResponder];
    
    return YES;
    
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
    
    if (textField == self.authenticationField)
        [self.authenticationEnabledSwitch setOn:([[textField.text stringByReplacingCharactersInRange:range withString:string] length] > 0)
                                       animated:YES];
    
    return YES;
    
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
    
    [self.nameField resignFirstResponder];
    [self.authenticationField resignFirstResponder];
    
}

@end

//
//  AirFloatPairViewController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatDAAPPairer.h"    
#import <UIKit/UIKit.h>

@interface AirFloatPairViewController : UIViewController <AirFloatDAAPPairerDelegate> {
    
    AirFloatDAAPPairer* _pairer;
    
}

@property (retain, nonatomic) IBOutlet UIView *passcodeView;
@property (retain, nonatomic) IBOutlet UILabel *passcodeLetter1;
@property (retain, nonatomic) IBOutlet UILabel *passcodeLetter2;
@property (retain, nonatomic) IBOutlet UILabel *passcodeLetter3;
@property (retain, nonatomic) IBOutlet UILabel *passcodeLetter4;

- (IBAction)cancelButtonTouchUpInside:(id)sender;

@end

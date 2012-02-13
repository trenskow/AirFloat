//
//  AirFloatViewController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "AirFloatImageView.h"
#import <UIKit/UIKit.h>

@interface AirFloatViewController : UIViewController {
    
    NSString* _previousPlayedAlbum;
    
}

@property (retain, nonatomic) IBOutlet UIView *infoViews;
@property (retain, nonatomic) IBOutlet UILabel *applicationStatusLabel;
@property (retain, nonatomic) IBOutlet UILabel *trackTitleLabel;
@property (retain, nonatomic) IBOutlet UILabel *artistNameLabel;
@property (retain, nonatomic) IBOutlet AirFloatImageView *artworkImageView;
@property (retain, nonatomic) IBOutlet AirFloatImageView *flippedArtworkImageView;
@property (retain, nonatomic) IBOutlet UIButton *playButton;
@property (retain, nonatomic) IBOutlet UIButton *nextButton;
@property (retain, nonatomic) IBOutlet UIButton *prevButton;

- (IBAction)infoViewTabGestureRecognized:(id)sender;
- (IBAction)playButtonTouchUpInside:(id)sender;
- (IBAction)nextButtonTouchUpInside:(id)sender;
- (IBAction)prevButtonTouchUpInside:(id)sender;
- (IBAction)betaFeedbackTouchUpInside:(id)sender;

@end

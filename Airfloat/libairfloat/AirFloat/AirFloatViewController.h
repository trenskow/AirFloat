//
//  AirFloatViewController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AirFloatServerController.h"


@interface AirFloatViewController : UIViewController {
    
    UISwitch* _serverSwitch;
    UIImageView* _statusLightImage;
    UIImageView* _noWifiImage;
    UIButton* _prevButton;
    UIButton* _playButton;
    UIButton* _nextButton;
    
    UIImage* _nowPlayingArtwork;
    
    AirFloatServerController* _serverController;
    
}

@property (nonatomic, retain) IBOutlet UISwitch* serverSwitch;
@property (nonatomic, retain) IBOutlet UIImageView* statusLightImage;
@property (nonatomic, retain) IBOutlet UIImageView* noWifiImage;
@property (nonatomic, retain) IBOutlet UILabel *noWifiLabel;
@property (nonatomic, retain) IBOutlet UIButton* prevButton;
@property (nonatomic, retain) IBOutlet UIButton* playButton;
@property (nonatomic, retain) IBOutlet UIButton* nextButton;
@property (nonatomic, retain) IBOutlet UIView *nowPlayingView;
@property (nonatomic, retain) IBOutlet UIImageView *artworkImageView;
@property (nonatomic, retain) IBOutlet UILabel *artistLabel;
@property (nonatomic, retain) IBOutlet UILabel *albumLabel;
@property (nonatomic, retain) IBOutlet UILabel *trackLabel;
@property (nonatomic, retain) IBOutlet UIView *displayOnView;
@property (nonatomic, retain) IBOutlet UIView *displayOffView;

- (IBAction)serverSwitchValueChanged:(id)sender;
- (IBAction)prevButtonPresset:(id)sender;
- (IBAction)playButtonPresset:(id)sender;
- (IBAction)nextButtonPresset:(id)sender;

@end

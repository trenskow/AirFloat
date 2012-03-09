//
//  AirFloatViewController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import <UIKit/UIKit.h>

@interface AirFloatViewController : UIViewController <UITableViewDelegate, UITableViewDataSource> {
    
    BOOL _playing;
    NSString* _previousPlayedAlbum;
    
    NSDictionary* _nowPlaying;
    
    BOOL _displayArtist;
    BOOL _displayAlbum;
    
    BOOL _contentOffsetSet;
    
    BOOL _canPausePlay;
    
    NSUInteger _previousPlayedIndex;
    
}

@property (retain, nonatomic) IBOutlet UIView *infoViews;
@property (retain, nonatomic) IBOutlet UILabel *applicationStatusLabel;
@property (retain, nonatomic) IBOutlet UILabel *trackTitleLabel;
@property (retain, nonatomic) IBOutlet UILabel *artistNameLabel;
@property (retain, nonatomic) IBOutlet AirFloatImageView *artworkImageView;
@property (retain, nonatomic) IBOutlet AirFloatImageView *flippedArtworkImageView;
@property (retain, nonatomic) IBOutlet UIView *swipeView;
@property (retain, nonatomic) IBOutlet UIButton *playButton;
@property (retain, nonatomic) IBOutlet UIButton *nextButton;
@property (retain, nonatomic) IBOutlet UIButton *prevButton;
@property (retain, nonatomic) IBOutlet UIButton *pairButton;
@property (retain, nonatomic) IBOutlet AirFloatBarButton *playlistButton;
@property (retain, nonatomic) IBOutlet UITableView *playlistTableView;
@property (retain, nonatomic) IBOutlet AirFloatAdView *adView;
@property (retain, nonatomic) IBOutlet UIView *bottomView;
@property (retain, nonatomic) IBOutlet UIView *topView;
@property (retain, nonatomic) IBOutlet UIView *wifiView;
@property (retain, nonatomic) IBOutlet UILabel *airfloatLabel;

- (IBAction)tapGestureRecognized:(id)sender;
- (IBAction)swipeGestureRecognized:(id)sender;
- (IBAction)playButtonTouchUpInside:(id)sender;
- (IBAction)nextButtonTouchUpInside:(id)sender;
- (IBAction)prevButtonTouchUpInside:(id)sender;
- (IBAction)betaFeedbackButtonTouchUpInside:(id)sender;
- (IBAction)pairButtonTouchUpInside:(id)sender;
- (IBAction)playlistButtonTouchUpInside:(id)sender;

@end

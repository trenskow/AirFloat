//
//  AirFloatViewController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatGenericViewController.h"

@interface AirFloatReceivingViewController : UIViewController <UITableViewDelegate, UITableViewDataSource, UIScrollViewDelegate, AirFloatTableViewDelegate, UITextFieldDelegate, AirFloatGenericViewController> {
    
    BOOL _playing;
    NSString* _previousPlayedAlbum;
    
    NSDictionary* _nowPlaying;
    NSDictionary* _nowPlayingFiltered;
    
    NSInteger _songCellStyle;
    
    BOOL _contentOffsetSet;
    BOOL _ignoreScroll;
    
    BOOL _canPausePlay;
    
    dispatch_queue_t _filterQueue;
    NSString* _currentFilterQuery;
    BOOL _filtering;
    
    UIView* _playlistTopBackgroundView;
    
}

@property (retain, nonatomic) IBOutlet UIView *infoViews;
@property (retain, nonatomic) IBOutlet UILabel *applicationStatusLabel;
@property (retain, nonatomic) IBOutlet UILabel *trackTitleLabel;
@property (retain, nonatomic) IBOutlet UILabel *artistNameLabel;
@property (retain, nonatomic) IBOutlet AirFloatImageView *artworkImageView;
@property (retain, nonatomic) IBOutlet UIImageView *flippedArtworkImageView;
@property (retain, nonatomic) IBOutlet UIView *swipeView;
@property (retain, nonatomic) IBOutlet UIButton *playButton;
@property (retain, nonatomic) IBOutlet UIButton *nextButton;
@property (retain, nonatomic) IBOutlet UIButton *prevButton;
@property (retain, nonatomic) IBOutlet UIButton *pairButton;
@property (retain, nonatomic) IBOutlet AirFloatBarButton *playlistButton;
@property (retain, nonatomic) IBOutlet AirFloatTableView *playlistTableView;
@property (retain, nonatomic) IBOutlet UIView *bottomView;
@property (retain, nonatomic) IBOutlet UIView *topView;
@property (retain, nonatomic) IBOutlet UIImageView *searchFieldBackground;
@property (retain, nonatomic) IBOutlet UIButton *searchClearButton;
@property (retain, nonatomic) IBOutlet UITextField *searchTextField;

- (IBAction)tapGestureRecognized:(id)sender;
- (IBAction)swipeGestureRecognized:(id)sender;
- (IBAction)playButtonTouchUpInside:(id)sender;
- (IBAction)nextButtonTouchUpInside:(id)sender;
- (IBAction)prevButtonTouchUpInside:(id)sender;
- (IBAction)pairButtonTouchUpInside:(id)sender;
- (IBAction)playlistButtonTouchUpInside:(id)sender;
- (IBAction)searchClearButtonTouchUpInside:(id)sender;
- (IBAction)searchTextFieldEditingChanged:(UITextField *)sender;

@end

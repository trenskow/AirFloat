//
//  AirFloatViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <MediaPlayer/MediaPlayer.h>

#import "TestFlight.h"

#import "AirFloatAdditions.h"
#import "AirFloatSongCell.h"
#import "AirFloatDAAPClient.h"
#import "AirFloatPairViewController.h"
#import "AirFloatAppDelegate.h"
#import "AirFloatViewController.h"

@interface AirFloatViewController (Private)

- (void)_failedFindingDaap;
- (void)_didPairClient;
- (void)_setInfoAppearence:(BOOL)visible;
- (void)_updateNowPlayingInfoCenter;
- (void)_updateArtwork:(UIImage*)image;
- (void)_updateApplicationStatus;
- (void)_playlistUpdated:(NSNotification*)notification;
- (void)_trackInfoUpdated:(NSNotification*)notification;
- (void)_metadataUpdated:(NSNotification*)notification;
- (void)_clientDisconnected;
- (void)_updatePlaybackStatus:(BOOL)playing;
- (void)_playbackStatusUpdated:(NSNotification*)notification;
- (void)_clientIsControllable;

@end

@implementation AirFloatViewController

#pragma mark - Property Implementation

@synthesize infoViews;
@synthesize applicationStatusLabel;
@synthesize trackTitleLabel;
@synthesize artistNameLabel;
@synthesize artworkImageView;
@synthesize flippedArtworkImageView;
@synthesize swipeView;
@synthesize playButton;
@synthesize nextButton;
@synthesize prevButton;
@synthesize pairButton;
@synthesize playlistButton;
@synthesize playlistTableView;
@synthesize adView;
@synthesize bottomView;
@synthesize topView;
@synthesize wifiView;
@synthesize airfloatLabel;

#pragma mark - Allocation / Deallocation / Load / Unload

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    [self becomeFirstResponder];
    
    self.flippedArtworkImageView.flipped = YES;
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_updateApplicationStatus) name:AirFloatServerControllerDidChangeStatusNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_trackInfoUpdated:) name:AirFloatTrackInfoUpdatedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_metadataUpdated:) name:AirFloatMetadataUpdatedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_clientDisconnected) name:AirFloatClientDisconnectedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_playbackStatusUpdated:) name:AirFloatDAAPPlaybackStatusUpdatedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_playlistUpdated:) name:AirFloatDAAPClientDidUpdatePlaylistNotification object:nil];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_clientIsControllable) name:AirFloatDAAPClientIsControllableNotification object:nil];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_didPairClient) name:AirFloatDAAPPairerDidPairNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_failedFindingDaap) name:AirFloatServerControllerFailedFindingDAAPNoification object:nil];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_updateApplicationStatus) name:UIApplicationWillEnterForegroundNotification object:nil];
    
    [self.artworkImageView addTapGestureRecognizerWithTarget:self action:@selector(tapGestureRecognized:)];
    
    [self.swipeView addTapGestureRecognizerWithTarget:self action:@selector(tapGestureRecognized:)];
    [self.swipeView addSwipeGestureRecognizerWithDirection:UISwipeGestureRecognizerDirectionLeft andTarget:self action:@selector(swipeGestureRecognized:)];
    [self.swipeView addSwipeGestureRecognizerWithDirection:UISwipeGestureRecognizerDirectionRight andTarget:self action:@selector(swipeGestureRecognized:)];
    
    self.adView.images = [NSArray arrayWithContentsOfFile:[NSMainBundle pathForResource:@"Images" ofType:@"plist"]];
    
    if ([UICurrentDevice.systemVersion doubleValue] < 5) {
        self.artistNameLabel.font = [UIFont fontWithName:@"HelveticaNeue" size:12];
        self.airfloatLabel.font = [UIFont fontWithName:@"HelveticaNeue" size:15];
    }
    
    [self _updateApplicationStatus];
    [self _clientDisconnected];
    
}

- (void)viewDidUnload {
    
    [[UIApplication sharedApplication] endReceivingRemoteControlEvents];
    
    [NSDefaultNotificationCenter removeObserver:self];
    
    [self setApplicationStatusLabel:nil];
    [self setTrackTitleLabel:nil];
    [self setArtistNameLabel:nil];
    [self setArtworkImageView:nil];
    [self setInfoViews:nil];
    [self setFlippedArtworkImageView:nil];
    [self setPlayButton:nil];
    [self setNextButton:nil];
    [self setPrevButton:nil];
    [self setPairButton:nil];
    [self setSwipeView:nil];
    [self setPlaylistButton:nil];
    [self setPlaylistTableView:nil];
    [self setAdView:nil];
    [self setBottomView:nil];
    [self setTopView:nil];
    [self setWifiView:nil];
    [self setAirfloatLabel:nil];
    
    [super viewDidUnload];
    
}

- (void)dealloc {
    
    [_previousPlayedAlbum release];
    
    [applicationStatusLabel release];
    [trackTitleLabel release];
    [artistNameLabel release];
    [artworkImageView release];
    [infoViews release];
    [flippedArtworkImageView release];
    [playButton release];
    [nextButton release];
    [prevButton release];    
    [pairButton release];
    [swipeView release];
    [playlistButton release];
    [playlistTableView release];
    [adView release];    
    [bottomView release];
    [topView release];
    [wifiView release];
    [airfloatLabel release];
    
    [super dealloc];
    
}

#pragma mark - Remote Control Handlers

- (BOOL)canBecomeFirstResponder {
    
    return YES;
    
}

- (void)remoteControlReceivedWithEvent:(UIEvent *)event {
    
    if (event.type == UIEventTypeRemoteControl)
        switch (event.subtype) {
            case UIEventSubtypeRemoteControlTogglePlayPause:
                [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackPlayPauseNotification object:nil];
                break;
            case UIEventSubtypeRemoteControlNextTrack:
                [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackNextNotification object:nil];
                break;
            case UIEventSubtypeRemoteControlPreviousTrack:
                [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackPrevNotification object:nil];
                break;
            default:
                break;
        }
    
}

#pragma mark - Action Methods
- (IBAction)tapGestureRecognized:(id)sender {
    
    [self _setInfoAppearence:(self.infoViews.alpha == 0.0)];
    
}

- (IBAction)swipeGestureRecognized:(id)sender {
    
    switch (((UISwipeGestureRecognizer*)sender).direction) {
        case UISwipeGestureRecognizerDirectionRight:
            [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackNextNotification object:nil];
            break;
        case UISwipeGestureRecognizerDirectionLeft:
            [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackPrevNotification object:nil];
            break;            
        default:
            break;
    }
    
}

- (IBAction)playButtonTouchUpInside:(id)sender {
    
    [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackPlayPauseNotification object:nil];
    
    if (_canPausePlay) {
        _playing = !_playing;
        [self.playButton setImage:[UIImage imageNamed:(_playing ? @"Pause.png" : @"Play.png")] forState:UIControlStateNormal];
    }
    
}

- (IBAction)nextButtonTouchUpInside:(id)sender {

    [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackNextNotification object:nil];

}

- (IBAction)prevButtonTouchUpInside:(id)sender {

    [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaybackPrevNotification object:nil];

}

- (IBAction)betaFeedbackButtonTouchUpInside:(id)sender {
    
    [TestFlight openFeedbackView];
    
}

- (IBAction)pairButtonTouchUpInside:(id)sender {
    
    [self presentModalViewController:[[[AirFloatPairViewController alloc] init] autorelease] animated:YES];
    
}

- (IBAction)playlistButtonTouchUpInside:(id)sender {
    
    BOOL display = self.playlistTableView.hidden;
    
    if (display) {
        self.playlistTableView.alpha = 0.0;
        self.playlistTableView.hidden = NO;
    }
    
    [UIView animateWithDuration:0.3
                          delay:0.0
                        options:UIViewAnimationCurveEaseInOut
                     animations:^{
                         self.playlistTableView.alpha = (display ? 1.0 : 0.0);
                     } completion:^(BOOL finished) {
                         if (!display)
                             self.playlistTableView.hidden = YES;
                     }];
                
}

#pragma mark - Private Methods

- (void)_failedFindingDaap {
    
    if ([AirFloatSharedAppDelegate.serverController.connectedUserAgent rangeOfString:@"iTunes"].location != NSNotFound && self.pairButton.hidden == YES) {
        
        self.pairButton.alpha = 0.0;
        self.pairButton.hidden = NO;
        [UIView animateWithDuration:0.3
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut
                         animations:^{
                             self.pairButton.alpha = 1.0;
                         } completion:NULL];
        
    }
    
}

- (void)_didPairClient {
    
    self.pairButton.hidden = YES;
    
}

- (void)_setInfoAppearence:(BOOL)visible {
    
    CGRect artworkImageRect = self.artworkImageView.frame;
    CGRect flippedArtworkImageRect = self.flippedArtworkImageView.frame;
    
    if ((!visible && artworkImageRect.origin.y == 44) || (visible && artworkImageRect.origin.y > 44)) {
        
        artworkImageRect.origin.y = (visible ? 44 : round(((self.view.bounds.size.height - 20) - artworkImageRect.size.height) / 2) + 10);
        
        flippedArtworkImageRect.origin.y = (visible ? 0 : 44 - artworkImageRect.origin.y);
        
        [UIView animateWithDuration:0.5
                              delay:0.0
                            options:UIViewAnimationOptionCurveEaseOut
                         animations:^{
                             
                             self.infoViews.alpha = (visible ? 1.0 : 0.0);
                             self.artworkImageView.frame = artworkImageRect;
                             self.flippedArtworkImageView.frame = flippedArtworkImageRect;
                             
                         } completion:NULL];

    }
    
}

- (void)_updateNowPlayingInfoCenter {
    
    if ([self.trackTitleLabel.text length] > 0 && NSClassFromString(@"MPNowPlayingInfoCenter")) {
        
        NSMutableDictionary* nowPlayingDictionary = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                                     self.trackTitleLabel.text, MPMediaItemPropertyTitle,
                                                     self.artistNameLabel.text, MPMediaItemPropertyArtist,
                                                     _previousPlayedAlbum, MPMediaItemPropertyAlbumTitle, nil];
        
        if (self.artworkImageView.fullsizeImage)
            [nowPlayingDictionary setObject:[[[MPMediaItemArtwork alloc] initWithImage:self.artworkImageView.fullsizeImage] autorelease] forKey:MPMediaItemPropertyArtwork];
        
        [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nowPlayingDictionary;
        
    } else
        [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;
    
}

- (void)_updateArtwork:(UIImage*)image {
    
    self.flippedArtworkImageView.image = image;
    self.artworkImageView.image = image;
    
    [self _updateNowPlayingInfoCenter];
    
}

- (void)_updateApplicationStatus {
    
    BOOL needsWifi = (AirFloatSharedAppDelegate.serverController.status == kAirFloatServerControllerNeedsWifiStatus);
    BOOL shouldDisplayApplication = (AirFloatSharedAppDelegate.serverController.status == kAirFloatServerControllerReceivingStatus);
    BOOL isVisible = !self.swipeView.hidden;
    
    self.infoViews.hidden = self.artworkImageView.hidden = needsWifi;
    
    [UIView animateWithDuration:1.0
                          delay:0.0
                        options:UIViewAnimationCurveEaseOut
                     animations:^{
                         self.wifiView.alpha = (needsWifi ? 1.0 : 0.0);
                     }
                     completion:NULL];
    
    if (needsWifi)
        [self.adView stopAnimation];
    else
        if (shouldDisplayApplication != isVisible) {
            
            if (shouldDisplayApplication) {
                [self _updateArtwork:nil];
                
                self.artworkImageView.alpha = 0.0;
                self.artworkImageView.hidden = NO;
                self.artworkImageView.transform = CGAffineTransformMakeScale(0.5, 0.5);
            } else
                [self.adView startAnimation];
            
            CGRect topEndFrame = self.topView.frame;
            CGRect bottomEndFrame = self.bottomView.frame;
            
            topEndFrame.origin.y = (shouldDisplayApplication ? 0.0 : -116);
            bottomEndFrame.origin.y = (shouldDisplayApplication ? self.view.frame.size.height - 96 : self.view.frame.size.height);
            
            if (shouldDisplayApplication)
                [self.adView stopAnimation];

                [UIView animateWithDuration:0.5
                                  delay:0.0
                                options:UIViewAnimationCurveEaseOut
                             animations:^{
                                 
                                 self.topView.frame = topEndFrame;
                                 self.bottomView.frame = bottomEndFrame;
                                 
                             } completion:^(BOOL finished) {
                                 
                                 self.swipeView.hidden = !shouldDisplayApplication;
                                 
                             }];
            
            [UIView animateWithDuration:1.0
                                  delay:0.0 
                                options:UIViewAnimationCurveEaseOut
                             animations:^{
                                 
                                 self.playlistTableView.alpha = self.artworkImageView.alpha = (shouldDisplayApplication ? 1.0 : 0.0);
                                 self.playlistTableView.transform = self.artworkImageView.transform = (shouldDisplayApplication ? CGAffineTransformMakeScale(1.0, 1.0) : CGAffineTransformMakeScale(1.5, 1.5));
                                 
                             } completion:^(BOOL finished) {
                                 
                                 if (!shouldDisplayApplication) {
                                     
                                     [self _setInfoAppearence:YES];
                                     self.artworkImageView.hidden = YES;
                                     self.playlistTableView.hidden = YES;
                                     self.playlistTableView.transform = CGAffineTransformMakeScale(1.0, 1.0);
                                     
                                 }
                                 
                             }];
            
        } else if (!shouldDisplayApplication)
            [self.adView startAnimation];

}

#pragma mark - Notification Handlers

- (void)_playlistUpdated:(NSNotification *)notification {
    
    NSArray* items = [[notification userInfo] objectForKey:kAirFloatDAAPPlaylistItemsKey];
    BOOL hasItems = ([items count] > 0);
        
    if (self.playlistButton.hidden == hasItems) {
        
        if (hasItems) {
            self.playlistButton.alpha = 0.0;
            self.playlistButton.hidden = NO;
        }
        
        [UIView animateWithDuration:0.3
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut | UIViewAnimationOptionAllowUserInteraction
                         animations:^{
                             self.playlistButton.alpha = (hasItems ? 1.0 : 0.0);
                         } completion:^(BOOL finished) {
                             if (!hasItems)
                                 self.playlistButton.hidden = YES;
                         }];
        
    }
    
    [_nowPlaying release];
    _nowPlaying = [[notification userInfo] retain];
    
    _displayArtist = _displayAlbum = NO;
    
    if (hasItems) {
        
        NSString* firstArtist = [NSNull ensureNonNil:[[items objectAtIndex:0] objectForKey:kAirFloatDAAPItemArtistNameKey]];
        NSString* firstAlbum = [NSNull ensureNonNil:[[items objectAtIndex:0] objectForKey:kAirFloatDAAPItemAlbumNameKey]];
        
        [items enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            
            NSString* artist = [NSNull ensureNonNil:[obj objectForKey:kAirFloatDAAPItemArtistNameKey]];
            NSString* album = [NSNull ensureNonNil:[obj objectForKey:kAirFloatDAAPItemAlbumNameKey]];
            
            _displayArtist = _displayArtist | ![artist isEqualToString:firstArtist];
            _displayAlbum = _displayAlbum | ![album isEqualToString:firstAlbum];
            
        }];
        
    }
    
    [self.playlistTableView reloadData];
    
    NSUInteger newNowPlayingIndex = [[[notification userInfo] objectForKey:kAirFloatDAAPPlaylistItemsPlayingSongIndexKey] integerValue];
    
    if (newNowPlayingIndex != _previousPlayedIndex) {
        
        NSUInteger found = NO;
        for (NSIndexPath* indexPath in [self.playlistTableView indexPathsForVisibleRows]) {
            CGRect cellFrame = ((UITableViewCell*)[self.playlistTableView cellForRowAtIndexPath:indexPath]).frame;
            cellFrame.origin.y -= self.playlistTableView.contentOffset.y;
            if (indexPath.row == newNowPlayingIndex) {
                if (cellFrame.origin.y < 0)
                    [self.playlistTableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionTop animated:YES];
                if (cellFrame.origin.y + cellFrame.size.height > self.playlistTableView.frame.size.height)
                    [self.playlistTableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionBottom animated:YES];
                found = YES;
                break;
            }
        }
        
        if (!found && newNowPlayingIndex < [[_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsKey] count])
            [self.playlistTableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:newNowPlayingIndex inSection:0] atScrollPosition:UITableViewScrollPositionMiddle animated:YES];
        
    }
    
    _previousPlayedIndex = newNowPlayingIndex;
    
}

- (void)_trackInfoUpdated:(NSNotification *)notification {
    
    if (AirFloatSharedAppDelegate.serverController.status == kAirFloatServerControllerReceivingStatus) {
        
        NSString* trackTitle = [notification.userInfo objectForKey:kAirFloatTrackInfoTrackTitleKey];
        NSString* artistName = [notification.userInfo objectForKey:kAirFloatTrackInfoArtistNameKey];
        
        NSString* albumName = [notification.userInfo objectForKey:kAirFloatTrackInfoAlbumNameKey];
        
        if (([[albumName stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] length] == 0 ||
             ![_previousPlayedAlbum isEqualToString:albumName]) && 
            !([self.trackTitleLabel.text isEqualToString:trackTitle] && [self.artistNameLabel.text isEqualToString:artistName]))
            [self _updateArtwork:nil];
        
        self.trackTitleLabel.text = trackTitle;
        self.artistNameLabel.text = artistName;    
        
        [_previousPlayedAlbum release];
        _previousPlayedAlbum = [albumName retain];
        
        [self _updateNowPlayingInfoCenter];
        
    }
    
}

- (void)_metadataUpdated:(NSNotification *)notification {
    
    if (AirFloatSharedAppDelegate.serverController.status == kAirFloatServerControllerReceivingStatus) {
        
        NSString* contentType = [notification.userInfo objectForKey:kAirFloatMetadataContentType];
        
        if ([contentType isEqualToString:@"image/jpeg"] || [contentType isEqualToString:@"image/png"])
            [self _updateArtwork:[UIImage imageWithData:[notification.userInfo objectForKey:kAirFloatMetadataDataKey]]];
        
    }
    
}

- (void)_clientDisconnected {
    
    self.trackTitleLabel.text = self.artistNameLabel.text = @"";
    
    self.playButton.alpha = self.nextButton.alpha = self.prevButton.alpha = 0.0;
    self.playButton.enabled = self.nextButton.enabled = self.prevButton.enabled = NO;
    
    self.pairButton.hidden = self.playlistButton.hidden = YES;
    
    _canPausePlay = NO;

}

- (void)_updatePlaybackStatus:(BOOL)playing {
    
    if (AirFloatSharedAppDelegate.serverController.status == kAirFloatServerControllerReceivingStatus) {
        
        [self.playButton setImage:[UIImage imageNamed:(playing ? @"Pause.png" : @"Play.png")] forState:UIControlStateNormal];
        
        if (!playButton.enabled) {
            
            CGRect prevEndFrame = self.prevButton.frame;
            CGRect nextEndFrame = self.nextButton.frame;
            
            CGRect prevStartFrame = prevEndFrame;
            CGRect nextStartFrame = nextEndFrame;
            
            prevStartFrame.origin.x -= 50;
            nextStartFrame.origin.x += 50;
            
            self.prevButton.frame = prevStartFrame;
            self.nextButton.frame = nextStartFrame;
            
            self.playButton.enabled = self.nextButton.enabled = self.prevButton.enabled = YES;
            
            [UIView animateWithDuration:0.3
                                  delay:0.0 
                                options:UIViewAnimationOptionCurveEaseOut | UIViewAnimationOptionAllowUserInteraction
                             animations:^{
                                 self.playButton.alpha = self.nextButton.alpha = self.prevButton.alpha = 1.0;
                                 self.prevButton.frame = prevEndFrame;
                                 self.nextButton.frame = nextEndFrame;
                             } completion:NULL];
            
        }
        
    }    
    
}

- (void)_playbackStatusUpdated:(NSNotification *)notification {
    
    if (AirFloatSharedAppDelegate.serverController.status == kAirFloatServerControllerReceivingStatus) {
        
        _canPausePlay = YES;
        
        [self _updatePlaybackStatus:([[notification.userInfo objectForKey:kAirFloatDAAPPlaybackStatusKey] integerValue] == kAirFloatDAAPPlaybackStatusPlaying)];
        
        if (((AirFloatDAAPClient*)[notification object]).clientType == kAirFloatDAAPClientTypeDACP && self.pairButton.hidden == YES)
            [self _failedFindingDaap];
        
    }
    
}

- (void)_clientIsControllable {
    
    [self _updatePlaybackStatus:YES];
    
}

#pragma mark - Table View Data Source Methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    
    return 1;
    
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    
    return [[_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsKey] count];
    
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    return [AirFloatSongCell cellForTableView:tableView];
    
}

#pragma mark - Table View Delegate Methods

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    return (_displayAlbum ? 57 : 44);
    
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    AirFloatSongCell* songCell = (AirFloatSongCell*)cell;
    
    NSUInteger nowPlayingIndex = [[_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsPlayingSongIndexKey] integerValue];
    
    NSArray* items = [_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsKey];
    NSDictionary* song = [items objectAtIndex:indexPath.row];
    
    songCell.displayAlbum = _displayAlbum;
    songCell.displayArtist = _displayArtist;
    
    [songCell adjustTrackNumberToWidth:[[NSString stringWithFormat:@"%d.", [items count]] sizeWithFont:songCell.trackNumberLabel.font].width];
    
    songCell.trackNumberLabel.text = [NSString stringWithFormat:@"%d.", indexPath.row + 1];
    songCell.trackNameLabel.text = [song objectForKey:kAirFloatDAAPItemNameKey];
    
    NSUInteger duration = [[song objectForKey:kAirFloatDAAPItemDurationKey] integerValue] / 1000;
    songCell.timeLabel.text = [NSString stringWithFormat:@"%d:%02d", duration / 60, duration % 60];
    
    songCell.artistNameLabel.text = [song objectForKey:kAirFloatDAAPItemArtistNameKey];
    songCell.albumNameLabel.text = [song objectForKey:kAirFloatDAAPItemAlbumNameKey];
    
    songCell.nowPlayingIndicatorView.hidden = (nowPlayingIndex != indexPath.row);
    
    songCell.backgroundColor = (indexPath.row % 2 == 1 ? [UIColor colorWithWhite:1.0 alpha:0.08] : [UIColor colorWithWhite:0.0 alpha:0.08]);
    
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    if (_nowPlaying)
        [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaySpecificNotification 
                                                   object:self 
                                                 userInfo:[NSDictionary dictionaryWithObject:[[[_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsKey] objectAtIndex:indexPath.row] objectForKey:kAirFloatDAAPItemIdenfifierKey] 
                                                                                      forKey:kAirFloatDAAPPlaySpecificIdentifierKey]];
    
}

@end

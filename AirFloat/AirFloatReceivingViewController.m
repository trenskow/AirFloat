//
//  AirFloatViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <MediaPlayer/MediaPlayer.h>

#import "AirFloatAdditions.h"
#import "AirFloatSongCell.h"
#import "AirFloatDAAPClient.h"
#import "AirFloatPairViewController.h"
#import "AirFloatAppDelegate.h"
#import "AirFloatServerController.h"
#import "AirFloatReceivingViewController.h"

#define kAirFloatPlaylistIdleTime 30.0

@interface AirFloatReceivingViewController (Private)

@property (nonatomic,assign,setter=_setPlaylistVisible:) BOOL _playlistVisible;

- (void)_setPlaylistVisible:(BOOL)_playlistVisible animated:(BOOL)animated slow:(BOOL)slowAnimated;
- (void)_playlistTimedOut;
- (void)_filter;
- (void)_keyboardWillAppearOrDisapper:(NSNotification*)notification;
- (void)_setViewVisibility:(BOOL)visible animated:(BOOL)animated;
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
- (void)_playbackStatusUpdated:(NSNotification*)notification;
- (void)_clientIsControllable;

@end

@implementation AirFloatReceivingViewController

#pragma mark - Allocation / Deallocation / Load / Unload

- (void)_addToView:(UIView*)view {
    
    [view addObserver:self forKeyPath:@"frame" options:NSKeyValueObservingOptionNew context:nil];
    for (UIView* subview in view.subviews)
        [self _addToView:subview];
    
}

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    [self becomeFirstResponder];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_trackInfoUpdated:) name:AirFloatTrackInfoUpdatedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_metadataUpdated:) name:AirFloatMetadataUpdatedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_clientDisconnected) name:AirFloatClientDisconnectedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_playbackStatusUpdated:) name:AirFloatDAAPPlaybackStatusUpdatedNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_playlistUpdated:) name:AirFloatDAAPClientDidUpdatePlaylistNotification object:nil];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_clientIsControllable) name:AirFloatDAAPClientIsControllableNotification object:nil];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_didPairClient) name:AirFloatDAAPPairerDidPairNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_failedFindingDaap) name:AirFloatServerControllerFailedFindingDAAPNoification object:nil];
    
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_keyboardWillAppearOrDisapper:) name:UIKeyboardWillShowNotification object:nil];
    [NSDefaultNotificationCenter addObserver:self selector:@selector(_keyboardWillAppearOrDisapper:) name:UIKeyboardWillHideNotification object:nil];
    
    [self.artworkImageView addTapGestureRecognizerWithTarget:self action:@selector(tapGestureRecognized:)];
    
    [self.swipeView addTapGestureRecognizerWithTarget:self action:@selector(tapGestureRecognized:)];
    [self.swipeView addSwipeGestureRecognizerWithDirection:UISwipeGestureRecognizerDirectionLeft andTarget:self action:@selector(swipeGestureRecognized:)];
    [self.swipeView addSwipeGestureRecognizerWithDirection:UISwipeGestureRecognizerDirectionRight andTarget:self action:@selector(swipeGestureRecognized:)];
    
    if ([UICurrentDevice.systemVersion doubleValue] < 5)
        self.artistNameLabel.font = [UIFont fontWithName:@"HelveticaNeue" size:12];
    
    self.searchFieldBackground.image = [self.searchFieldBackground.image stretchableImageWithLeftCapWidth:18 topCapHeight:0];
    
    [self.searchClearButton addObserver:self forKeyPath:@"hidden" options:NSKeyValueObservingOptionNew context:nil];
    
    [self _addToView:self.view];
    
    _filterQueue = dispatch_queue_create("com.AirFloat.PlaylistFilterQueue", DISPATCH_QUEUE_SERIAL);
    
    _playlistTopBackgroundView = [[UIView alloc] initWithFrame:CGRectZero];
    
    _playlistTopBackgroundView.backgroundColor = [UIColor colorWithWhite:0.0 alpha:0.34];
    _playlistTopBackgroundView.opaque = NO;
    
    [self.playlistTableView addSubview:_playlistTopBackgroundView];
    
    self.playlistTableView.contentInset = UIEdgeInsetsMake(-44, 0, 0, 0);
    
    self.playlistTableView.showsShortCutScrollBar = YES;
    self.playlistTableView.shortCutScrollBarColor = [UIColor whiteColor];
    
    [self.pairButton addObserver:self forKeyPath:@"hidden" options:NSKeyValueObservingOptionNew context:NULL];
    
    // Simulate client disconnect to setup disconnected state.
    
    [self _clientDisconnected];
    
    [self _setViewVisibility:NO animated:NO];
    
}

- (void)viewDidUnload {
    
    [self.searchClearButton removeObserver:self forKeyPath:@"hidden"];
    [self.pairButton removeObserver:self forKeyPath:@"hidden"];
    
    [[UIApplication sharedApplication] endReceivingRemoteControlEvents];
    
    [NSDefaultNotificationCenter removeObserver:self];
    
    dispatch_release(_filterQueue);
    
    [_playlistTopBackgroundView removeFromSuperview];
    [_playlistTopBackgroundView release];
    _playlistTopBackgroundView = nil;
    
    [_nowPlaying release];
    [_nowPlayingFiltered release];
    
    _nowPlaying = _nowPlayingFiltered = nil;
    
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
    [self setBottomView:nil];
    [self setTopView:nil];
    [self setSearchFieldBackground:nil];
    [self setSearchTextField:nil];
    [self setSearchClearButton:nil];
    
    [super viewDidUnload];
    
}

- (void)dealloc {
    
    [_nowPlaying release];
    [_nowPlayingFiltered release];
    
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
    [bottomView release];
    [topView release];
    [searchFieldBackground release];
    [searchTextField release];
    [searchClearButton release];
    
    [super dealloc];
    
}

#pragma mark - Public Properties

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
@synthesize bottomView;
@synthesize topView;
@synthesize searchFieldBackground;
@synthesize searchClearButton;
@synthesize searchTextField;

#pragma mark - Private Properties

- (BOOL)_playlistVisible {
    
    return !self.playlistTableView.hidden;
    
}

- (void)_setPlaylistVisible:(BOOL)_playlistVisible {
    
    [self _setPlaylistVisible:_playlistVisible animated:YES slow:NO];
    
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
    
    [self _setViewVisibility:apparent animated:animated];
    
}

- (void)setAppereance:(BOOL)apparent {
    
    [self _setViewVisibility:apparent animated:YES];
    
}

#pragma mark - Remote Control Handlers

- (BOOL)canBecomeFirstResponder {
    
    return YES;
    
}

- (void)remoteControlReceivedWithEvent:(UIEvent *)event {
    
    if (event.type == UIEventTypeRemoteControl)
        switch (event.subtype) {
            case UIEventSubtypeRemoteControlPlay:
            case UIEventSubtypeRemoteControlPause:
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
    
    //[self _setInfoAppearence:(self.infoViews.alpha == 0.0)];
    
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

- (IBAction)pairButtonTouchUpInside:(id)sender {
    
    [self presentModalViewController:[[[AirFloatPairViewController alloc] init] autorelease] animated:YES];
    
}

- (IBAction)playlistButtonTouchUpInside:(id)sender {
    
    self._playlistVisible = !self._playlistVisible;
    
}

- (IBAction)searchClearButtonTouchUpInside:(id)sender {
    
    if ([[self.searchTextField.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] length] > 0) {
        self.searchTextField.text = @"";
        [self searchTextFieldEditingChanged:self.searchTextField];
    } else
        [self.searchTextField resignFirstResponder];    
    
}

- (IBAction)searchTextFieldEditingChanged:(UITextField *)sender {
    
    NSString* query = [sender.text stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
    
    if ([query length] > 0) {
        
        [_currentFilterQuery release];
        _currentFilterQuery = [sender.text copy];
        
        if (!_filtering)
            [self _filter];
        
    } else {
        
        [_currentFilterQuery release];
        _currentFilterQuery = nil;
        
        [_nowPlayingFiltered release];
        _nowPlayingFiltered = [_nowPlaying retain];
        
        [self.playlistTableView reloadData];
    }
    
}

#pragma mark - Private Methods

- (void)_setPlaylistVisible:(BOOL)_playlistVisible animated:(BOOL)animated slow:(BOOL)slowAnimated {
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_playlistTimedOut) object:nil];
    
    if (self._playlistVisible != _playlistVisible) {
        
        if (_playlistVisible) {
            
            self.playlistTableView.alpha = 0.0;
            self.playlistTableView.hidden = NO;
            [self performSelector:@selector(_playlistTimedOut) withObject:nil afterDelay:kAirFloatPlaylistIdleTime];
            
        }
        
        [self.artworkImageView setBlur:_playlistVisible animated:animated slow:slowAnimated];
        
        [UIView animateWithDuration:(animated ? (slowAnimated ? 3.0 : 0.3) : 0.0)
                              delay:0.0
                            options:UIViewAnimationCurveEaseInOut
                         animations:^{
                             self.playlistTableView.alpha = (_playlistVisible ? 1.0 : 0.0);
                         } completion:^(BOOL finished) {
                             if (!_playlistVisible)
                                 self.playlistTableView.hidden = YES;
                         }];
        
    }
    
}

- (void)_playlistTimedOut {
    
    [self _setPlaylistVisible:NO animated:YES slow:YES];
    
}

- (void)_filter {
    
    dispatch_async(_filterQueue, ^{
        
        __block NSString* query = nil;
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            _filtering = YES;
            query = [[_currentFilterQuery lowercaseString] copy];
        });
        
        NSArray* queries = [[query stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]] componentsSeparatedByString:@" "];
        
        NSMutableDictionary* newFilteredDictionary = [_nowPlaying mutableCopy];
        NSMutableArray* filteredItems = [NSMutableArray array];
        
        for (NSDictionary* item in [_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsKey]) {
            NSMutableString* searchString = [NSMutableString stringWithString:[[item objectForKey:kAirFloatDAAPItemNameKey] lowercaseString]];
            
            if (_songCellStyle == kAirFloatSongCellStyleFull)
                [searchString appendFormat:@" %@ %@", [[item objectForKey:kAirFloatDAAPItemArtistNameKey] lowercaseString], [[item objectForKey:kAirFloatDAAPItemAlbumNameKey] lowercaseString]];
            
            BOOL found = ([queries count] > 0);
            for (NSString* subQuery in queries)
                found = found & ([searchString rangeOfString:subQuery].location != NSNotFound);
            
            if (found)
                [filteredItems addObject:item];
            
        }
        
        [newFilteredDictionary setObject:filteredItems
                                  forKey:kAirFloatDAAPPlaylistItemsKey];
        
        dispatch_sync(dispatch_get_main_queue(), ^{
            
            [_nowPlayingFiltered release];
            _nowPlayingFiltered = newFilteredDictionary;
            
            [self.playlistTableView reloadData];
            
            _filtering = NO;
            
            if (![[_currentFilterQuery lowercaseString] isEqualToString:query])
                [self _filter];
            
            [query release];
            
        });
        
    });
    
}

- (void)_keyboardWillAppearOrDisapper:(NSNotification *)notification {
    
    BOOL keyboardWillShow = [[notification name] isEqualToString:UIKeyboardWillShowNotification];
    
    NSInteger curve = [[[notification userInfo] objectForKey:UIKeyboardAnimationCurveUserInfoKey] integerValue];
    NSTimeInterval duration = [[[notification userInfo] objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue];
    
    CGRect viewFrame = self.view.frame;
    viewFrame.origin.y = (keyboardWillShow ? -76 : 20);
    
    CGRect keyboardEndFrame = [[[notification userInfo] objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    
    CGRect playlistTableViewFrame = self.playlistTableView.frame;
    playlistTableViewFrame.size.height = (keyboardWillShow ? self.view.height - keyboardEndFrame.size.height : 268);
    
    [UIView animateWithDuration:duration
                          delay:curve
                        options:UIViewAnimationCurveEaseOut
                     animations:^{
                         self.view.frame = viewFrame;
                         self.playlistTableView.frame = playlistTableViewFrame;
                     } completion:NULL];
    
}

- (void)_setViewVisibility:(BOOL)visible animated:(BOOL)animated {
    
    if (visible == self.view.hidden) {
        
        if (visible) {
            
            self.view.hidden = NO;
            
            self.artworkImageView.alpha = 0.0;
            self.artworkImageView.hidden = NO;
            self.artworkImageView.transform = CGAffineTransformMakeScale(0.5, 0.5);
            
        }
        
        CGRect topEndFrame = self.topView.frame;
        CGRect bottomEndFrame = self.bottomView.frame;
        
        topEndFrame.origin.y = (visible ? 0.0 : -116);
        bottomEndFrame.origin.y = (visible ? self.view.frame.size.height - 96 : self.view.frame.size.height);
                
        [UIView animateWithDuration:(animated ? 0.5 : 0.0)
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut
                         animations:^{
                             
                             self.topView.frame = topEndFrame;
                             self.bottomView.frame = bottomEndFrame;
                             
                         } completion:^(BOOL finished) {
                             
                             self.swipeView.hidden = !visible;
                             
                         }];
        
        [UIView animateWithDuration:(animated ? 1.0 : 0.0)
                              delay:0.0 
                            options:UIViewAnimationCurveEaseOut
                         animations:^{
                             
                             self.playlistTableView.alpha = self.artworkImageView.alpha = (visible ? 1.0 : 0.0);
                             self.playlistTableView.transform = self.artworkImageView.transform = (visible ? CGAffineTransformMakeScale(1.0, 1.0) : CGAffineTransformMakeScale(1.5, 1.5));
                             
                         } completion:^(BOOL finished) {
                             
                             if (!visible) {
                                 
                                 [self _setInfoAppearence:YES];
                                 self.artworkImageView.hidden = YES;
                                 self.playlistTableView.hidden = YES;
                                 self.playlistTableView.transform = CGAffineTransformMakeScale(1.0, 1.0);
                                 
                                 self.view.hidden = YES;
                                 
                                 self.artworkImageView.blur = NO;
                                 self.artworkImageView.generateBlurredImage = NO;
                                 self.artworkImageView.image = nil;
                                 [self _updateArtwork:nil];

                             }
                             
                         }];
    }
    
}

- (void)_failedFindingDaap {
    
    if ([AirFloatiOSSharedAppDelegate.serverController.connectedUserAgent rangeOfString:@"iTunes"].location != NSNotFound && self.pairButton.hidden == YES) {
        
        /*
         
         Pairing has currently been disabled due to bug.
         
        self.pairButton.alpha = 0.0;
        self.pairButton.hidden = NO;
        [UIView animateWithDuration:0.3
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut
                         animations:^{
                             self.pairButton.alpha = 1.0;
                         } completion:NULL];
         
         */
        
    }
    
}

- (void)_didPairClient {
    
    self.pairButton.hidden = YES;
    
}

- (void)_setInfoAppearence:(BOOL)visible {
    
    CGRect startFrame = self.artworkImageView.frame;
    CGRect endFrame = CGRectMake(-10, 34, 340, self.view.bounds.size.height - 120);
    
    if ((!visible && startFrame.origin.y == 34) || (visible && startFrame.origin.y == -10)) {
        
        if (!visible) {
            endFrame = self.view.bounds;
            endFrame = CGRectMake(endFrame.origin.x - 10, endFrame.origin.y - 10, endFrame.size.width + 20, endFrame.size.height + 20);
        }
        
        CGFloat centerDiff = (endFrame.origin.y + (endFrame.size.height / 2)) - (startFrame.origin.y + (startFrame.size.height / 2));
        
        CGRect flippedArtworkImageRect = self.flippedArtworkImageView.frame;
        
        flippedArtworkImageRect.origin.y -= centerDiff;
        
        [UIView animateWithDuration:0.5
                              delay:0.0
                            options:UIViewAnimationOptionCurveEaseOut
                         animations:^{
                             
                             self.infoViews.alpha = (visible ? 1.0 : 0.0);
                             self.artworkImageView.frame = endFrame;
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
    
    self.artworkImageView.image = image;
    
    [self _updateNowPlayingInfoCenter];
    
}

#pragma mark - Public Methods / Overridden Methods

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    
    if ([keyPath isEqualToString:@"hidden"]) {
        if (object == self.searchClearButton)
            self.searchTextField.width = 231 - (self.searchClearButton.hidden ? 0 : self.searchClearButton.width);
        else if (object == self.pairButton && !self.pairButton.hidden && [AirFloatiOSSharedAppDelegate.serverController.connectedUserAgent rangeOfString:@"iTunes"].location == NSNotFound)
            self.pairButton.hidden = NO;
    }
    
}

#pragma mark - Notification Handlers

- (void)_playlistUpdated:(NSNotification *)notification {
    
    NSArray* items = [[notification userInfo] objectForKey:kAirFloatDAAPPlaylistItemsKey];
    BOOL hasItems = ([items count] > 0);
        
    if (self.playlistButton.hidden == hasItems) {
        
        if (hasItems) {
            self.playlistButton.alpha = 0.0;
            self.pairButton.hidden = YES;
            self.playlistButton.hidden = NO;
            self.artworkImageView.generateBlurredImage = YES;
        }
        
        [UIView animateWithDuration:0.3
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut | UIViewAnimationOptionAllowUserInteraction
                         animations:^{
                             self.playlistButton.alpha = (hasItems ? 1.0 : 0.0);
                         } completion:^(BOOL finished) {
                             if (!hasItems) {
                                 self.playlistButton.hidden = YES;
                                 self.artworkImageView.generateBlurredImage = NO;
                             }
                         }];
        
    }
    
    [_nowPlaying release];
    _nowPlaying = [[notification userInfo] retain];
    
    _songCellStyle = kAirFloatSongCellStyleSimple;
    
    if (hasItems) {
        
        NSString* firstArtist = [NSNull ensureNonNil:[[items objectAtIndex:0] objectForKey:kAirFloatDAAPItemArtistNameKey]];
        NSString* firstAlbum = [NSNull ensureNonNil:[[items objectAtIndex:0] objectForKey:kAirFloatDAAPItemAlbumNameKey]];
        
        [items enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            
            NSString* artist = [NSNull ensureNonNil:[obj objectForKey:kAirFloatDAAPItemArtistNameKey]];
            NSString* album = [NSNull ensureNonNil:[obj objectForKey:kAirFloatDAAPItemAlbumNameKey]];
            
            if (![artist isEqualToString:firstArtist] || ![album isEqualToString:firstAlbum]) {
                _songCellStyle = kAirFloatSongCellStyleFull;
                *stop = YES;
            }
            
        }];
        
    }
    
    if (![[notification userInfo] objectForKey:kAirFloatDAAPPlaylistItemsPreviouslyPlayedItemKey]) {
        
        dispatch_release(_filterQueue);
        _filterQueue = dispatch_queue_create("com.AirFloat.PlaylistFilterQueue", DISPATCH_QUEUE_SERIAL);
        
        [_nowPlayingFiltered release];
        _nowPlayingFiltered = [_nowPlaying retain];
        
        self.searchTextField.text = @"";
        self.searchClearButton.hidden = YES;
        
        [self.playlistTableView reloadData];
        
        [self.playlistTableView setShowsShortCutScrollBar:(self.playlistTableView.contentSize.height > self.playlistTableView.bounds.size.height * 3) animated:YES];
        
        if (self.playlistTableView.contentOffset.y == 0)
            self.playlistTableView.contentOffset = CGPointMake(0, 44);
        
    }

    for (NSIndexPath* indexPath in [self.playlistTableView indexPathsForVisibleRows]) {
        AirFloatSongCell* cell = (AirFloatSongCell*) [self.playlistTableView cellForRowAtIndexPath:indexPath];
        cell.nowPlayingIndicatorView.hidden = ([[notification userInfo] objectForKey:kAirFloatDAAPPlaylistItemsPlayingItemKey] != [[_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey] objectAtIndex:indexPath.row]);
    }    
    
    if (playlistTableView.contentInset.top != 0) {
        
        if ([notification.userInfo objectForKey:kAirFloatDAAPPlaylistItemsPlayingItemKey] != [notification.userInfo objectForKey:kAirFloatDAAPPlaylistItemsPreviouslyPlayedItemKey]) {
            
            NSInteger newNowPlayingIndex = [[_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey] indexOfObject:[[notification userInfo] objectForKey:kAirFloatDAAPPlaylistItemsPlayingItemKey]];
            
            if (newNowPlayingIndex != NSNotFound) {
                
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
                
                if (!found && newNowPlayingIndex < [[_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey] count])
                    [self.playlistTableView scrollToRowAtIndexPath:[NSIndexPath indexPathForRow:newNowPlayingIndex inSection:0] atScrollPosition:UITableViewScrollPositionMiddle animated:YES];
                
                [self.playlistTableView reloadShortCutScrollBar];
                
            }
            
        }

    }
    
}

- (void)_trackInfoUpdated:(NSNotification *)notification {
    
    if (AirFloatiOSSharedAppDelegate.serverController.status == kAirFloatServerControllerStatusReceiving) {
        
        NSString* trackTitle = [notification.userInfo objectForKey:kAirFloatTrackInfoTrackTitleKey];
        NSString* artistName = [notification.userInfo objectForKey:kAirFloatTrackInfoArtistNameKey];
        
        NSString* albumName = [notification.userInfo objectForKey:kAirFloatTrackInfoAlbumNameKey];
        
        self.trackTitleLabel.text = trackTitle;
        self.artistNameLabel.text = artistName;    
        
        [_previousPlayedAlbum release];
        _previousPlayedAlbum = [albumName retain];
        
        [self _updateNowPlayingInfoCenter];
        
    }
    
}

- (void)_metadataUpdated:(NSNotification *)notification {
    
    if (AirFloatiOSSharedAppDelegate.serverController.status == kAirFloatServerControllerStatusReceiving) {
        
        NSString* contentType = [notification.userInfo objectForKey:kAirFloatMetadataContentType];
        
        if ([contentType isEqualToString:@"image/jpeg"] || [contentType isEqualToString:@"image/png"])
            [self _updateArtwork:[UIImage imageWithData:[notification.userInfo objectForKey:kAirFloatMetadataDataKey]]];
        else if ([contentType isEqualToString:@"image/none"])
            [self _updateArtwork:nil];
        
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
    
    if (AirFloatiOSSharedAppDelegate.serverController.status == kAirFloatServerControllerStatusReceiving) {
        
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
    
    if (AirFloatiOSSharedAppDelegate.serverController.status == kAirFloatServerControllerStatusReceiving) {
        
        _canPausePlay = YES;
        
        [self _updatePlaybackStatus:([[notification.userInfo objectForKey:kAirFloatDAAPPlaybackStatusKey] integerValue] == kAirFloatDAAPPlaybackStatusPlaying)];
        
        if (((AirFloatDAAPClient*)[notification object]).clientType == kAirFloatDAAPClientTypeDACP && self.pairButton.hidden == YES)
            [self _failedFindingDaap];
        
    }
    
}

- (void)_clientIsControllable {
    
    [self _updatePlaybackStatus:YES];
    
}

#pragma mark - Text Field Delegate Methods

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField {
    
    self.searchClearButton.hidden = NO;
    if (self.playlistTableView.contentOffset.y != 0.0) {
        _ignoreScroll = YES;
        [self.playlistTableView setContentOffset:CGPointZero animated:YES];
    }
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_playlistTimedOut) object:nil];
    
    return YES;
    
}

- (BOOL)textFieldShouldEndEditing:(UITextField *)textField {
    
    self.searchClearButton.hidden = ([textField.text length] == 0);
    
    [self performSelector:@selector(_playlistTimedOut) withObject:nil afterDelay:kAirFloatPlaylistIdleTime];
    
    return YES;
    
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    
    [self.searchTextField resignFirstResponder];
    
    return YES;
    
}

#pragma mark - Table View Data Source Methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    
    return 1;
    
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    
    return [[_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey] count];
    
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    return [AirFloatSongCell cellForTableView:tableView];
    
}

#pragma mark - Table View Scroll View Delegate Methods

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate {
    
    if (scrollView.contentOffset.y < 0) {
        
        [UIView beginAnimations:nil context:NULL];
        [UIView setAnimationDuration:0.3];
        
        scrollView.contentInset = UIEdgeInsetsMake(0, 0, 0, 0);
        
        [UIView commitAnimations];
        
        [self.playlistTableView setShowsShortCutScrollBar:NO animated:YES];
        
    }
        
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
    
    if (scrollView.contentOffset.y > 44) {
        scrollView.contentInset = UIEdgeInsetsMake(-44, 0, 0, 0);
        if (self.playlistTableView.contentSize.height > self.playlistTableView.bounds.size.height * 3)
            [self.playlistTableView setShowsShortCutScrollBar:YES animated:YES];
    }
    
    _playlistTopBackgroundView.frame = CGRectMake(0, MIN(0, scrollView.contentOffset.y), scrollView.frame.size.width, MAX(0, -scrollView.contentOffset.y));
    
    if (!_ignoreScroll) {
        if ([self.searchTextField isFirstResponder])
            [self.searchTextField resignFirstResponder];
    } else if (scrollView.contentOffset.y == 0.0)
        _ignoreScroll = NO;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_playlistTimedOut) object:nil];
    [self performSelector:@selector(_playlistTimedOut) withObject:nil afterDelay:kAirFloatPlaylistIdleTime];
    
}

#pragma mark - Table View Delegate Methods

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    return [AirFloatSongCell sizeForStyle:_songCellStyle].height;
    
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    AirFloatSongCell* songCell = (AirFloatSongCell*)cell;
    
    NSArray* items = [_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey];
    NSDictionary* song = [items objectAtIndex:indexPath.row];
    
    songCell.style = _songCellStyle;
    
    CGFloat cellHeight = [AirFloatSongCell sizeForStyle:_songCellStyle].height;    
    songCell.infoView.frame = CGRectMake(0, 0, (cellHeight * [items count] > self.playlistTableView.bounds.size.height * 3 ? 320 - self.playlistTableView.shortCutScrollBarWidth : 320), cellHeight);
    
    // Adjust to unfiltered count
    [songCell adjustTrackNumberToWidth:[[NSString stringWithFormat:@"%d.", [[_nowPlaying objectForKey:kAirFloatDAAPPlaylistItemsKey] count]] sizeWithFont:songCell.trackNumberLabel.font].width];
    
    songCell.trackNumberLabel.text = [NSString stringWithFormat:@"%d.", indexPath.row + 1];
    songCell.trackNameLabel.text = [song objectForKey:kAirFloatDAAPItemNameKey];
    
    NSUInteger duration = [[song objectForKey:kAirFloatDAAPItemDurationKey] integerValue] / 1000;
    songCell.timeLabel.text = [NSString stringWithFormat:@"%d:%02d", duration / 60, duration % 60];
    
    songCell.artistNameLabel.text = [song objectForKey:kAirFloatDAAPItemArtistNameKey];
    songCell.albumNameLabel.text = [song objectForKey:kAirFloatDAAPItemAlbumNameKey];
    
    NSDictionary* nowPlayingItem = [_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsPlayingItemKey];
    songCell.nowPlayingIndicatorView.hidden = (!nowPlayingItem || song != nowPlayingItem);
    
    songCell.backgroundColor = (indexPath.row % 2 == 1 ? [UIColor colorWithWhite:1.0 alpha:0.08] : [UIColor clearColor]);
    
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    if ([self.searchTextField isFirstResponder])
        [self.searchTextField resignFirstResponder];
    
    if (_nowPlayingFiltered)
        [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPlaySpecificNotification 
                                                   object:self 
                                                 userInfo:[NSDictionary dictionaryWithObject:[[[_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey] objectAtIndex:indexPath.row] objectForKey:kAirFloatDAAPItemIdenfifierKey] 
                                                                                      forKey:kAirFloatDAAPPlaySpecificIdentifierKey]];
    
}

#pragma mark - Table View AirFloat Delegate Methods

- (UIImage*)airFloatTableView:(AirFloatTableView *)tableView imageForPageIndicator:(NSInteger)page {
    
    NSDictionary* nowPlayingItem = [_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsPlayingItemKey];
    NSInteger index;
    if (nowPlayingItem && (index = [[_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey] indexOfObject:nowPlayingItem]) != NSNotFound &&
        [tableView pageForRowAtIndexPath:[NSIndexPath indexPathForRow:index inSection:0]] == page)
        return [UIImage imageNamed:@"NowPlayingIndicatorSmall.png"];
    
    return nil;
    
}

- (CGPoint)airFloatTableView:(AirFloatTableView *)tableView offsetForPage:(NSInteger)page withSuggestedOffset:(CGPoint)offset {
    
    NSDictionary* nowPlayingItem = [_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsPlayingItemKey];
    NSInteger index;
    if (nowPlayingItem && (index = [[_nowPlayingFiltered objectForKey:kAirFloatDAAPPlaylistItemsKey] indexOfObject:nowPlayingItem]) != NSNotFound &&
        [tableView pageForRowAtIndexPath:[NSIndexPath indexPathForRow:index inSection:0]] == page) {
            
        CGRect rect = [tableView rectForRowAtIndexPath:[NSIndexPath indexPathForRow:index inSection:0]];
        
        return CGPointMake(0, MIN(MAX(rect.origin.y - ((tableView.bounds.size.height - rect.size.height) / 2), -tableView.contentInset.top), tableView.contentSize.height - tableView.bounds.size.height - tableView.contentInset.bottom));
        
    }
            
    return offset;
    
}

@end

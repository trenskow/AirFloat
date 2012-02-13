//
//  AirFloatViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <MediaPlayer/MediaPlayer.h>

#import "TestFlight.h"

#import "UIView+AirFloatAdditions.h"
#import "RTPReceiver.h"
#import "AirFloatAppDelegate.h"
#import "AirFloatViewController.h"

@interface AirFloatViewController (Private)

- (void)_updateNowPlayingInfoCenter;
- (void)_updateArtwork:(UIImage*)image;
- (void)_updateApplicationStatus;
- (void)_trackInfoUpdated:(NSNotification*)notification;
- (void)_metadataUpdated:(NSNotification*)notification;
- (void)_clientDisconnected:(NSNotification*)notification;
- (void)_playbackStatusUpdated:(NSNotification*)notification;

@end

@implementation AirFloatViewController
@synthesize infoViews;
@synthesize applicationStatusLabel;
@synthesize trackTitleLabel;
@synthesize artistNameLabel;
@synthesize artworkImageView;
@synthesize flippedArtworkImageView;
@synthesize playButton;
@synthesize nextButton;
@synthesize prevButton;

- (id)init {
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
        return (self = [self initWithNibName:@"AirFloatViewController~iPad" bundle:nil]);
    
    return (self = [self initWithNibName:@"AirFloatViewController" bundle:nil]);
    
}

- (BOOL)canBecomeFirstResponder {
    
    return YES;
    
}

- (void)viewDidLoad {
    
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    [self becomeFirstResponder];
    
    self.flippedArtworkImageView.flipped = YES;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_updateApplicationStatus) name:AirFloatServerControllerDidChangeStatus object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_trackInfoUpdated:) name:AirFloatTrackInfoUpdatedNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_metadataUpdated:) name:AirFloatMetadataUpdatedNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_clientDisconnected:) name:AirFloatClientDisconnectedNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_playbackStatusUpdated:) name:AirFloatPlaybackStatusUpdatedNotification object:nil];
    
    [self _updateApplicationStatus];
    
    [super viewDidLoad];
    
}

- (void)viewDidUnload {
    
    [[UIApplication sharedApplication] endReceivingRemoteControlEvents];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [self setApplicationStatusLabel:nil];
    [self setTrackTitleLabel:nil];
    [self setArtistNameLabel:nil];
    [self setArtworkImageView:nil];
    [self setInfoViews:nil];
    [self setFlippedArtworkImageView:nil];
    [self setPlayButton:nil];
    [self setNextButton:nil];
    [self setPrevButton:nil];
    
    [super viewDidUnload];
    
}

- (void)remoteControlReceivedWithEvent:(UIEvent *)event {
    
    if (event.type == UIEventTypeRemoteControl)
        switch (event.subtype) {
            case UIEventSubtypeRemoteControlTogglePlayPause:
                [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackPlayPauseNotification object:nil];
                break;
            case UIEventSubtypeRemoteControlNextTrack:
                [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackNextNotification object:nil];
                break;
            case UIEventSubtypeRemoteControlPreviousTrack:
                [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackPrevNotification object:nil];
                break;
            default:
                break;
        }
    
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
    
    [super dealloc];
    
}

#pragma mark Action Methods
- (IBAction)infoViewTabGestureRecognized:(id)sender {
    
    CGRect artworkImageRect = self.artworkImageView.frame;
    CGRect flippedArtworkImageRect = self.flippedArtworkImageView.frame;
    BOOL willAppear = (self.infoViews.alpha == 0.0);
    
    artworkImageRect.origin.y = (willAppear ? 44 : artworkImageRect.origin.y = round(((self.view.bounds.size.height) - artworkImageRect.size.height) / 2) - 10);
    
    flippedArtworkImageRect.origin.y = (willAppear ? 0 : 44 - artworkImageRect.origin.y);
    
    [UIView animateWithDuration:0.5
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         
                         self.infoViews.alpha = (willAppear ? 1.0 : 0.0);
                         self.artworkImageView.frame = artworkImageRect;
                         self.flippedArtworkImageView.frame = flippedArtworkImageRect;
                         
                     } completion:NULL];
    
}

- (IBAction)playButtonTouchUpInside:(id)sender {
    
    [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackPlayPauseNotification object:nil];
    
}

- (IBAction)nextButtonTouchUpInside:(id)sender {

    [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackNextNotification object:nil];

}

- (IBAction)prevButtonTouchUpInside:(id)sender {

    [[NSNotificationCenter defaultCenter] postNotificationName:AirFloatPlaybackPrevNotification object:nil];

}

- (IBAction)betaFeedbackTouchUpInside:(id)sender {
    
    [TestFlight openFeedbackView];
    
}

#pragma mark Private Methods

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
    
    AirFloatServerController* serverController = AirFloatSharedAppDelegate.serverController;
    
    if (!serverController.wifiReachability.isAvailable)
        self.applicationStatusLabel.text = @"Connect to Wireless Network";
    else if (!serverController.isRunning)
        self.applicationStatusLabel.text = @"Broke";
    else if (serverController.isRecording)
        self.applicationStatusLabel.text = @"Playing";
    else
        self.applicationStatusLabel.text = @"Ready";
    
    if (!serverController.hasClientConnected)
        self.trackTitleLabel.text = self.artistNameLabel.text = @"";
    
}

- (void)_trackInfoUpdated:(NSNotification *)notification {
    
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

- (void)_metadataUpdated:(NSNotification *)notification {
    
    NSString* contentType = [notification.userInfo objectForKey:kAirFloatMetadataContentType];
    
    if ([contentType isEqualToString:@"image/jpeg"] || [contentType isEqualToString:@"image/png"])
        [self _updateArtwork:[UIImage imageWithData:[notification.userInfo objectForKey:kAirFloatMetadataDataKey]]];
    
}

- (void)_clientDisconnected:(NSNotification*)notification {
    
    void* sender = [[notification.userInfo objectForKey:kAirFloatSenderOriginKey] pointerValue];
    
    RTPReceiver* streamingReceiver = RTPReceiver::getStreamingReceiver();
    
    void* currentConnection = NULL;
    
    if (streamingReceiver)
        currentConnection = streamingReceiver->getConnection();
    
    if (currentConnection == NULL || currentConnection == sender) {
        self.trackTitleLabel.text = self.artistNameLabel.text = @"";
        [self _updateArtwork:nil];
    }
    
    self.playButton.alpha = self.nextButton.alpha = self.prevButton.alpha = 0.0;
    self.playButton.enabled = self.nextButton.enabled = self.prevButton.enabled = NO;
    
}

- (void)_playbackStatusUpdated:(NSNotification *)notification {
    
    [self.playButton setImage:[UIImage imageNamed:([[notification.userInfo objectForKey:kAirFloatPlaybackStatusKey] integerValue] == kAirFloatPlaybackStatusPlaying ? @"Pause.png" : @"Play.png")] forState:UIControlStateNormal];
    
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
                            options:UIViewAnimationOptionCurveEaseOut
                         animations:^{
                             self.playButton.alpha = self.nextButton.alpha = self.prevButton.alpha = 1.0;
                             self.prevButton.frame = prevEndFrame;
                             self.nextButton.frame = nextEndFrame;
                         } completion:NULL];
        
    }
    
}

@end

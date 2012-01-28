//
//  AirFloatViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 7/13/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <MediaPlayer/MediaPlayer.h>

#import "RTPReceiver.h"
#import "AirFloatAppDelegate.h"
#import "AirFloatViewController.h"

@interface AirFloatViewController (Private)

- (void)_updateNowPlaying:(UIImage*)artwork;
- (void)_setImage:(NSData *)imageData;

@end

@implementation AirFloatViewController
@synthesize nowPlayingView = _nowPlayingView;
@synthesize artworkImageView = _artworkImageView;
@synthesize artistLabel = _artistLabel;
@synthesize albumLabel = _albumLabel;
@synthesize trackLabel = _trackLabel;
@synthesize displayOnView = _displayOnView;
@synthesize displayOffView = _displayOffView;

@synthesize serverSwitch=_serverSwitch, statusLightImage=_statusLightImage, noWifiImage=_noWifiImage, noWifiLabel = _noWifiLabel, prevButton=_prevButton, playButton=_playButton, nextButton=_nextButton;

- (id)init {
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
        return (self = [self initWithNibName:@"AirFloatViewController~iPad" bundle:nil]);
        
    return (self = [self initWithNibName:@"AirFloatViewController" bundle:nil]);
    
}

- (void)_updateNowPlaying:(UIImage*)artwork {
    
    [_nowPlayingArtwork release];
    _nowPlayingArtwork = [artwork retain];
    
    NSDictionary* dictionary = [NSDictionary dictionaryWithObjectsAndKeys:self.trackLabel.text, MPMediaItemPropertyTitle,
                                self.artistLabel.text, MPMediaItemPropertyArtist,
                                self.albumLabel.text, MPMediaItemPropertyAlbumTitle,
                                (artwork ? [[[MPMediaItemArtwork alloc] initWithImage:artwork] autorelease] : nil), MPMediaItemPropertyArtwork, nil];
    
    [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = dictionary;
    
}

- (void)_didReceiveServerControllerNotification:(NSNotification*)notification {
    
    if ([[notification name] isEqualToString:AirFloatServerStartedNotification])
        self.statusLightImage.image = [UIImage imageNamed:@"GreenLight.png"];
    if ([[notification name] isEqualToString:AirFloatClientSupportsPlayControlsNotification]) {
        
        self.prevButton.alpha = self.playButton.alpha = self.nextButton.alpha = 0.0;
        
        CGRect prevFinalFrame, prevStartFrame;
        CGRect nextFinalFrame, nextStartFrame;
        prevFinalFrame = prevStartFrame = self.prevButton.frame;
        nextFinalFrame = nextStartFrame = self.nextButton.frame;
        
        prevStartFrame.origin.x -= 100;
        nextStartFrame.origin.x += 100;
        
        self.prevButton.frame = prevStartFrame;
        self.nextButton.frame = nextStartFrame;
        
        self.prevButton.hidden = self.playButton.hidden = self.nextButton.hidden = NO;
        
        [UIView animateWithDuration:0.5
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut
                         animations:^(void) {
                             
                             self.prevButton.frame = prevFinalFrame;
                             self.nextButton.frame = nextFinalFrame;
                             
                             self.prevButton.alpha = self.playButton.alpha = self.nextButton.alpha = 1.0;
                             
                         } completion:NULL];
        
    } else if ([[notification name] isEqualToString:AirFloatClientConnectedNotification] || [[notification name] isEqualToString:AirFloatClientStoppedRecordingNotification]) {
        self.statusLightImage.image = [UIImage imageNamed:@"OrangeLight.png"];
        self.statusLightImage.hidden = NO;
    } else if ([[notification name] isEqualToString:AirFloatClientDisconnectedNotification]) {
        
        self.statusLightImage.image = [UIImage imageNamed:@"GreenLight.png"];
        
        self.trackLabel.text = self.albumLabel.text = self.artistLabel.text = @"";
        self.artworkImageView.image = [UIImage imageNamed:@"NoArtwork.png"];
        
        [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;
        
        [UIView animateWithDuration:0.5
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut
                         animations:^(void) {
                             
                             self.prevButton.alpha = self.playButton.alpha = self.nextButton.alpha = 0.0;                             
                             
                         } completion:^(BOOL finished) {
                             
                             self.prevButton.hidden = self.playButton.hidden = self.nextButton.hidden = YES;
                             
                         }];
        
    } else if ([[notification name] isEqualToString:AirFloatClientUpdatedMetadataNotification]) {
        
        NSData* data = [notification.userInfo objectForKey:kAirFloatClientMetadataData];
        NSString* contentType = [notification.userInfo objectForKey:kAirFloatClientMetadataContentType];
        
        if ([contentType isEqualToString:@"image/jpeg"]) {
            
            UIImage* image = [UIImage imageWithData:data];
            
            [self _updateNowPlaying:image];
            
            UIGraphicsBeginImageContextWithOptions(self.artworkImageView.bounds.size, YES, self.view.window.screen.scale);
            
            CGContextRef context = UIGraphicsGetCurrentContext();
            
            UIGraphicsPushContext(context);
            
            [image drawInRect:self.artworkImageView.bounds];
            
            UIGraphicsPopContext();
            
            self.artworkImageView.image = UIGraphicsGetImageFromCurrentImageContext();
            
            UIGraphicsEndImageContext();
            
        } else if ([contentType isEqualToString:@"image/none"]) {
            self.artworkImageView.image = [UIImage imageNamed:@"NoArtwork.png"];
            [self _updateNowPlaying:nil];
        } else if ([contentType isEqualToString:@"application/x-dmap-tagged"] && [data length] > 8) {
            
            UIImage* image = nil;
            
            if (![self.albumLabel.text isEqualToString:[notification.userInfo objectForKey:kAirFloatClientMetadataAlbum]])
                self.artworkImageView.image = [UIImage imageNamed:@"NoArtwork.png"];
            else
                image = [[_nowPlayingArtwork retain] autorelease];
            
            self.trackLabel.text = [notification.userInfo objectForKey:kAirFloatClientMetadataTrackTitle];
            self.artistLabel.text = [notification.userInfo objectForKey:kAirFloatClientMetadataArtistName];
            self.albumLabel.text = [notification.userInfo objectForKey:kAirFloatClientMetadataAlbum];
            
            [self _updateNowPlaying:image];
            
        }
        
    }
    
    self.displayOffView.hidden = !(self.displayOnView.hidden = ([self.trackLabel.text length] == 0));
    
}

- (BOOL)canBecomeFirstResponder {
    
    return YES;
    
}

- (void)remoteControlReceivedWithEvent:(UIEvent *)event {
    
    
}

- (void)viewDidLoad {
    
    _serverController = [((AirFloatAppDelegate*)[UIApplication sharedApplication].delegate).serverController retain];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_didReceiveServerControllerNotification:) name:nil object:_serverController];
    
    [self becomeFirstResponder];
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    
    self.trackLabel.text = self.artistLabel.text = self.albumLabel.text = @"";
    
    if (!_serverController.isWifiAvailable)
        self.noWifiImage.hidden = self.noWifiLabel.hidden = NO;
    
}

- (void)viewDidUnload {
    [self setDisplayOffView:nil];
    [self setDisplayOnView:nil];
    [self setArtworkImageView:nil];
    [self setTrackLabel:nil];
    [self setAlbumLabel:nil];
    [self setArtistLabel:nil];
    [self setNowPlayingView:nil];
    [self setNoWifiLabel:nil];
    
    [[UIApplication sharedApplication] endReceivingRemoteControlEvents];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [_serverController release];
    
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
        return YES;
    
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (IBAction)serverSwitchValueChanged:(id)sender {
    
    
}

- (IBAction)prevButtonPresset:(id)sender {
    
    [_serverController dacpPrev];
    
}

- (IBAction)playButtonPresset:(id)sender {
    
    [_serverController dacpPlay];
    
}

- (IBAction)nextButtonPresset:(id)sender {
    
    [_serverController dacpNext];
    
}

- (void)dealloc {
    
    [_nowPlayingArtwork release];
    
    [_noWifiLabel release];
    [_nowPlayingView release];
    [_artistLabel release];
    [_albumLabel release];
    [_trackLabel release];
    [_artworkImageView release];
    [_displayOnView release];
    [_displayOffView release];
    [super dealloc];
}
@end

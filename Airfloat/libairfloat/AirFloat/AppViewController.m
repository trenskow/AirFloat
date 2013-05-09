//
//  AppViewController.m
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

#import <MediaPlayer/MediaPlayer.h>

#import "webserverconnection.h"
#import "dacpclient.h"
#import "raopsession.h"

#import "UIImage+AirFloatAdditions.h"

#import "AirFloatAppDelegate.h"
#import "AirFloatAdView.h"
#import "SupportViewController.h"
#import "SettingsViewController.h"
#import "AppViewController.h"

@interface AppViewController () {
    
    raop_server_p _server;
    dacp_client_p _dacp_client;
    UIViewController* _overlaidViewController;
    UIImage* _artworkImage;
    NSString* _albumTitle;
    
}

@property (nonatomic, strong) IBOutlet UIButton* supportButton;
@property (nonatomic, strong) IBOutlet UIButton* settingsButton;
@property (nonatomic, strong) IBOutlet AirFloatAdView* adView;
@property (nonatomic, strong) IBOutlet UIView* topView;
@property (nonatomic, strong) IBOutlet UIView* bottomView;
@property (nonatomic, strong) IBOutlet UIImageView* artworkImageView;
@property (nonatomic, strong) IBOutlet UIImageView* blurredArtworkImageView;
@property (nonatomic, strong) IBOutlet UILabel* trackTitelLabel;
@property (nonatomic, strong) IBOutlet UILabel* artistNameLabel;
@property (nonatomic, strong) IBOutletCollection(UIButton) NSArray* controlButtons;
@property (nonatomic, strong) IBOutlet UIButton* playButton;
@property (nonatomic, strong) IBOutlet UIView* overlaidViewContainer;

- (void)clientStartedRecording;
- (void)clientEndedRecording;
- (void)clientEnded;
- (void)clientUpdatedArtwork:(UIImage *)image;
- (void)clientUpdatedTrackInfo:(NSString *)trackTitle artistName:(NSString *)artistName andAlbumTitle:(NSString *)albumTitle;
- (void)setDacpClient:(NSValue*)pointer;
- (void)updatePlaybackState;
- (void)updateControlsAvailability;

@end

UIBackgroundTaskIdentifier backgroundTask = 0;

void dacpClientControlsBecameAvailable(dacp_client_p client, void* ctx) {
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    [viewController performSelectorOnMainThread:@selector(updateControlsAvailability) withObject:nil waitUntilDone:NO];
    
}

void dacpClientPlaybackStateUpdated(dacp_client_p client, enum dacp_client_playback_state state, void* ctx) {
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    [viewController performSelectorOnMainThread:@selector(updatePlaybackState) withObject:nil waitUntilDone:NO];
    
}

void dacpClientControlsBecameUnavailable(dacp_client_p client, void* ctx) {
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    [viewController performSelectorOnMainThread:@selector(updateControlsAvailability) withObject:nil waitUntilDone:NO];
    
}

void clientStartedRecording(raop_session_p raop_session, void* ctx) {
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    dacp_client_p client = raop_session_get_dacp_client(raop_session);
    
    if (client != NULL) {
        
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
        
        [viewController performSelectorOnMainThread:@selector(setDacpClient:) withObject:[NSValue valueWithPointer:client] waitUntilDone:NO];
        
        [pool release];
        
        dacp_client_set_controls_became_available_callback(client, dacpClientControlsBecameAvailable, ctx);
        dacp_client_set_playback_state_changed_callback(client, dacpClientPlaybackStateUpdated, ctx);
        dacp_client_set_controls_became_unavailable_callback(client, dacpClientControlsBecameUnavailable, ctx);
        
    }
    
    [viewController performSelectorOnMainThread:@selector(clientStartedRecording) withObject:nil waitUntilDone:NO];
    
}

void clientEndedRecording(raop_session_p raop_session, void* ctx) {
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    if ([UIApplication sharedApplication].applicationState == UIApplicationStateBackground)
        backgroundTask = [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:nil];
    
    [viewController performSelectorOnMainThread:@selector(clientEndedRecording) withObject:nil waitUntilDone:NO];
    
}

void clientEnded(raop_session_p raop_session, void* ctx) {
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    [viewController performSelectorOnMainThread:@selector(clientEnded) withObject:nil waitUntilDone:NO];
    
}

void clientUpdatedArtwork(raop_session_p raop_session, const void* data, size_t data_size, const char* mime_type, void* ctx) {
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    UIImage* image = nil;
    
    if (strcmp(mime_type, "image/none") != 0) {
        NSData* imageData = [[NSData alloc] initWithBytes:data length:data_size];
        image = [[UIImage imageWithData:imageData] imageWithScale:[UIScreen mainScreen].scale];
        [imageData release];
    }
    
    [viewController performSelectorOnMainThread:@selector(clientUpdatedArtwork:) withObject:image waitUntilDone:NO];
    
    [pool release];
    
}

void clientUpdatedTrackInfo(raop_session_p raop_session, const char* title, const char* artist, const char* album, void* ctx) {
    
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
    AppViewController* viewController = (AppViewController*)ctx;
    
    NSString* trackTitle = [[NSString alloc] initWithCString:title encoding:NSUTF8StringEncoding];
    NSString* artistTitle = [[NSString alloc] initWithCString:artist encoding:NSUTF8StringEncoding];
    NSString* albumTitle = [[NSString alloc] initWithCString:album encoding:NSUTF8StringEncoding];
    
    NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:[viewController methodSignatureForSelector:@selector(clientUpdatedTrackInfo:artistName:andAlbumTitle:)]];
    [invocation setSelector:@selector(clientUpdatedTrackInfo:artistName:andAlbumTitle:)];
    [invocation setTarget:viewController];
    [invocation setArgument:&trackTitle atIndex:2];
    [invocation setArgument:&artistTitle atIndex:3];
    [invocation setArgument:&albumTitle atIndex:4];
    [invocation retainArguments];
    
    [invocation performSelectorOnMainThread:@selector(invoke) withObject:nil waitUntilDone:YES];
    
    [trackTitle release];
    [artistTitle release];
        
    [pool release];
    
}

void newServerSession(raop_server_p server, raop_session_p new_session, void* ctx) {
    
    raop_session_set_client_started_recording_callback(new_session, clientStartedRecording, ctx);
    raop_session_set_client_ended_recording_callback(new_session, clientEndedRecording, ctx);
    raop_session_set_client_updated_artwork_callback(new_session, clientUpdatedArtwork, ctx);
    raop_session_set_client_updated_track_info_callback(new_session, clientUpdatedTrackInfo, ctx);
    raop_session_set_ended_callback(new_session, clientEnded, ctx);
    
}

@implementation AppViewController

- (void)dealloc {
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    self.adView = nil;
    self.topView = self.bottomView = nil;
    self.artworkImageView = nil;
    self.trackTitelLabel = self.artistNameLabel = nil;
    
    [_artworkImage release];
    [_albumTitle release];
    
    [super dealloc];
    
}

- (raop_server_p)server {
    
    return _server;
    
}

- (void)setServer:(raop_server_p)server {
    
    [self willChangeValueForKey:@"server"];
    
    if (server) {
        raop_server_set_new_session_callback(server, newServerSession, self);
        [self.adView startAnimation];
    } else
        [self.adView stopAnimation];
    
    _server = server;
    [self didChangeValueForKey:@"server"];
    
}

- (void)updateBlurredArtwork {
    
    if (raop_server_is_recording(self.server))
        [NSThread detachNewThreadSelector:@selector(setAndScaleImage:) toTarget:self withObject:self.artworkImageView.image];
    
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    
    // OpenGL ES is not always ready at this point. Wait a sec.
    [self performSelector:@selector(updateBlurredArtwork) withObject:nil afterDelay:1.0];
    
}

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone)
        [self.adView setImages:[NSArray arrayWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"Images" ofType:@"plist"]]];
    else
        [self.adView setImages:[NSArray arrayWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"Images~ipad" ofType:@"plist"]]];
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad)
        self.artworkImageView.contentMode = UIViewContentModeScaleAspectFit;
    
    if ([[UIDevice currentDevice].systemVersion floatValue] >= 6) {
        
        self.blurredArtworkImageView = [[[UIImageView alloc] initWithFrame:self.artworkImageView.frame] autorelease];
        self.blurredArtworkImageView.autoresizingMask = self.artworkImageView.autoresizingMask;
        self.blurredArtworkImageView.contentMode = self.artworkImageView.contentMode;
        self.blurredArtworkImageView.clipsToBounds = self.artworkImageView.clipsToBounds;
        self.blurredArtworkImageView.alpha = 0.0;
        
        [self.artworkImageView.superview insertSubview:self.blurredArtworkImageView aboveSubview:self.artworkImageView];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(applicationDidBecomeActive:)
                                                     name:UIApplicationDidBecomeActiveNotification
                                                   object:nil];
        
    }
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(settingsUpdated:) name:SettingsUpdatedNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(batteryStateChanged:) name:UIDeviceBatteryStateDidChangeNotification object:nil];
    
    [UIDevice currentDevice].batteryMonitoringEnabled = YES;
    
    [self updateScreenIdleState];
    
}

- (void)viewDidUnload {
    
    [super viewDidUnload];
    
    self.adView = nil;
    self.topView = self.bottomView = nil;
    self.artworkImageView = nil;
    self.trackTitelLabel = self.artistNameLabel = nil;
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
}

- (void)viewDidAppear:(BOOL)animated {
    
    [super viewDidAppear:animated];
    
    if (self.server)
        [self.adView startAnimation];
    
}

- (void)displayViewControllerAsOverlay:(UIViewController*)viewController {
    
    _overlaidViewController = [viewController retain];
    _overlaidViewController.view.frame = self.overlaidViewContainer.bounds;
    [self.overlaidViewContainer addSubview:viewController.view];
    
    [_overlaidViewController viewWillAppear:YES];
    
    [UIView animateWithDuration:0.3
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         
                         if (self.server && raop_server_is_recording(self.server)) {
                             self.blurredArtworkImageView.alpha = 1.0;
                             self.artworkImageView.alpha = 0.5;
                         }
                         
                         self.overlaidViewContainer.alpha = 1.0;
                         
                     } completion:^(BOOL finished) {
                         
                         [_overlaidViewController viewDidAppear:YES];
                         
                     }];
    
}

- (void)dismissOverlayViewController {
    
    [_overlaidViewController viewWillDisappear:YES];
    
    [UIView animateWithDuration:0.3
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         
                         if (self.server && raop_server_is_recording(self.server)) {
                             self.blurredArtworkImageView.alpha = 0.0;
                             self.artworkImageView.alpha = 1.0;
                         }
                         
                         self.overlaidViewContainer.alpha = 0.0;
                         
                     } completion:^(BOOL finished) {
                         
                         while ([self.overlaidViewContainer.subviews count] > 0)
                             [[self.overlaidViewContainer.subviews lastObject] removeFromSuperview];
                         
                         [_overlaidViewController viewDidDisappear:YES];
                         
                         [_overlaidViewController release];
                         _overlaidViewController = nil;
                         
                     }];
    
}

- (IBAction)supportButtonTouchUpInside:(id)sender {
    
    BOOL visible = (_overlaidViewController != nil);
    
    if (!visible)
        [self displayViewControllerAsOverlay:[[[SupportViewController alloc] init] autorelease]];
    else
        [self dismissOverlayViewController];
    
    [self.supportButton setTitle:(!visible ? @"Close" : @"Support")
                        forState:UIControlStateNormal];
    
    self.settingsButton.userInteractionEnabled = visible;
    
    [UIView animateWithDuration:0.3
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.settingsButton.alpha = (!visible ? 0.0 : 1.);
                     } completion:nil];
    
}

- (IBAction)settingsButtonTouchUpInside:(id)sender {
    
    BOOL visible = (_overlaidViewController != nil);
    
    if (!visible)
        [self displayViewControllerAsOverlay:[[[SettingsViewController alloc] init] autorelease]];
    else
        [self dismissOverlayViewController];
    
    [self.settingsButton setTitle:(!visible ? @"Close" : @"Settings")
                         forState:UIControlStateNormal];
    
    self.supportButton.userInteractionEnabled = visible;
    
    [UIView animateWithDuration:0.3
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.supportButton.alpha = (!visible ? 0.0 : 1.);
                     } completion:nil];
    
}

- (void)updateNowPlayingInfoCenter {
    
    if (NSClassFromString(@"MPNowPlayingInfoCenter")) {
        
        if (raop_server_is_recording(self.server)) {
            
            NSMutableDictionary* nowPlayingInfo = [[NSMutableDictionary alloc] init];
            
            if (self.trackTitelLabel.text) {
                
                [nowPlayingInfo setObject:self.trackTitelLabel.text forKey:MPMediaItemPropertyTitle];
                if (self.artistNameLabel.text)
                    [nowPlayingInfo setObject:self.artistNameLabel.text forKey:MPMediaItemPropertyArtist];
                
                if (_albumTitle)
                    [nowPlayingInfo setObject:_albumTitle forKey:MPMediaItemPropertyAlbumTitle];
                
                if (_artworkImage)
                    [nowPlayingInfo setObject:[[[MPMediaItemArtwork alloc] initWithImage:_artworkImage] autorelease]
                                   forKey:MPMediaItemPropertyArtwork];
                
                [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nowPlayingInfo;
                
            }
            
            [nowPlayingInfo release];
            
        } else
            [MPNowPlayingInfoCenter defaultCenter].nowPlayingInfo = nil;
        
    }
    
}

- (BOOL)canBecomeFirstResponder {
    
    return YES;
    
}

- (void)clientStartedRecording {
    
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    [self becomeFirstResponder];
    
    CGRect topViewFrame = self.topView.frame;
    topViewFrame.size.height = 96;
    
    CGRect overlaidViewContainerFrame = self.overlaidViewContainer.frame;
    overlaidViewContainerFrame.origin.y = 47;
    overlaidViewContainerFrame.size.height = self.overlaidViewContainer.superview.bounds.size.height - 47;
    
    self.blurredArtworkImageView.alpha = (_overlaidViewController != nil ? 1.0 : 0.0);
    
    [UIView animateWithDuration:0.5
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.topView.frame = topViewFrame;
                         self.trackTitelLabel.alpha = self.artistNameLabel.alpha = 1.0;
                         self.overlaidViewContainer.frame = overlaidViewContainerFrame;
                     } completion:nil];
    
    [UIView animateWithDuration:1.0
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.artworkImageView.alpha = 1.0;
                     } completion:nil];
    
    [self.adView stopAnimation];
    
    self.trackTitelLabel.text = self.artistNameLabel.text = nil;
    
    [self updateControlsAvailability];
    
    [self updateScreenIdleState];
    
}

- (void)clientEndedRecording {
    
    _dacp_client = NULL;
    
    CGRect topViewFrame = self.topView.frame;
    topViewFrame.size.height = 48;
    
    CGRect overlaidViewContainerFrame = self.overlaidViewContainer.frame;
    overlaidViewContainerFrame.origin.y = 0;
    overlaidViewContainerFrame.size.height = self.overlaidViewContainer.superview.bounds.size.height;
    
    [UIView animateWithDuration:0.5
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.topView.frame = topViewFrame;
                         self.trackTitelLabel.alpha = self.artistNameLabel.alpha = 0.0;
                         self.overlaidViewContainer.frame = overlaidViewContainerFrame;
                     } completion:nil];
    
    [UIView animateWithDuration:1.0
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.artworkImageView.alpha = 0.0;
                         self.blurredArtworkImageView.alpha = 0.0;
                     } completion:^(BOOL finished) {
                         self.blurredArtworkImageView.alpha = 0.0;
                         self.artworkImageView.image = [UIImage imageNamed:@"NoArtwork.png"];
                         self.blurredArtworkImageView.image = nil;
                         [_artworkImage release];
                         _artworkImage = nil;
                         [_albumTitle release];
                         _albumTitle = nil;
                     }];
    
    [self.adView startAnimation];
    
    [self updateNowPlayingInfoCenter];
    
    [self updateScreenIdleState];
    
}

- (void)stopBackgroundTask {
    
    UIBackgroundTaskIdentifier identifier = backgroundTask;
    backgroundTask = 0;
    [[UIApplication sharedApplication] endBackgroundTask:identifier];
    
}

- (void)clientEnded {
    
    if ([UIApplication sharedApplication].applicationState == UIApplicationStateBackground && backgroundTask > 0) {
        raop_server_stop(self.server);
        [self performSelector:@selector(stopBackgroundTask) withObject:nil afterDelay:1.0];
    }
    
}

- (void)setAndScaleImage:(UIImage *)image {
    
    CGSize size = self.artworkImageView.bounds.size;
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        
        CGSize screenSize = [UIScreen mainScreen].bounds.size;
        size = CGSizeMake(MAX(screenSize.width, screenSize.height),
                          MAX(screenSize.width, screenSize.height));
        
    }
    
    UIImage* actualImage = [image imageAspectedFilledWithSize:size];
    UIImage* blurredImage = [actualImage imageGaussianBlurredWithRadius:10.0];
    
    [self.artworkImageView performSelectorOnMainThread:@selector(setImage:) withObject:actualImage waitUntilDone:NO];
    [self.blurredArtworkImageView performSelectorOnMainThread:@selector(setImage:) withObject:blurredImage waitUntilDone:NO];
    
}

- (void)clientUpdatedArtwork:(UIImage *)image {
    
    [self setAndScaleImage:(image ?: [UIImage imageNamed:@"NoArtwork.png"])];
    
    [_artworkImage release];
    _artworkImage = [image retain];
    
    [self updateNowPlayingInfoCenter];
    
}

- (void)clientUpdatedTrackInfo:(NSString *)trackTitle artistName:(NSString *)artistName andAlbumTitle:(NSString *)albumTitle {
    
    self.trackTitelLabel.text = trackTitle;
    self.artistNameLabel.text = artistName;
    
    [_albumTitle release];
    _albumTitle = albumTitle;
    
    [self updateNowPlayingInfoCenter];
    
}

- (void)setDacpClient:(NSValue*)pointer {
    
    _dacp_client = (dacp_client_p)[pointer pointerValue];
    
}

- (void)updatePlaybackState {
    
    if (_dacp_client != NULL) {
        
        bool playing = (dacp_client_get_playback_state(_dacp_client) == dacp_client_playback_state_playing);
        
        [self.playButton setImage:[UIImage imageNamed:(playing ? @"Pause.png" : @"Play.png")]
                         forState:UIControlStateNormal];
        
    }
    
}

- (void)updateControlsAvailability {
    
    BOOL isAvailable = (_dacp_client != NULL && dacp_client_is_available(_dacp_client));
    
    UIButton* lastControl = [self.controlButtons lastObject];
    
    if (isAvailable && lastControl.hidden) {
        for (UIButton* button in self.controlButtons) {
            button.alpha = 0.0;
            button.hidden = NO;
        }
    }
    
    [UIView animateWithDuration:1.0
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         
                         for (UIButton* button in self.controlButtons)
                             button.alpha = (isAvailable ? 1.0 : 0.0);
                         
                     } completion:^(BOOL finished) {
                         
                         for (UIButton* button in self.controlButtons)
                             button.hidden = !isAvailable;
                         
                     }];
    
    if (isAvailable)
        dacp_client_update_playback_state(_dacp_client);
    
}

- (IBAction)playNext:(id)sender {
    
    if (_dacp_client != NULL)
        dacp_client_next(_dacp_client);
    
}

- (IBAction)playPause:(id)sender {
    
    if (_dacp_client != NULL) {
        dacp_client_toggle_playback(_dacp_client);
        dacp_client_update_playback_state(_dacp_client);
    }
    
}

- (IBAction)playPrevious:(id)sender {
    
    if (_dacp_client != NULL)
        dacp_client_previous(_dacp_client);
    
}

- (void)remoteControlReceivedWithEvent:(UIEvent *)event {
    
    if (event.type == UIEventTypeRemoteControl)
        switch (event.subtype) {
            case UIEventSubtypeRemoteControlNextTrack:
                [self playNext:nil];
                break;
            case UIEventSubtypeRemoteControlPause:
            case UIEventSubtypeRemoteControlPlay:
            case UIEventSubtypeRemoteControlTogglePlayPause:
            case UIEventSubtypeRemoteControlStop:
                [self playPause:nil];
                break;
            case UIEventSubtypeRemoteControlPreviousTrack:
                [self playPrevious:nil];
                break;
            default:
                break;
        }
    
}

- (BOOL)shouldAutorotate {
    
    return ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad);
    
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation {
    
    return ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad || toInterfaceOrientation == UIDeviceOrientationPortrait);
    
}

- (void)updateScreenIdleState {
    
    BOOL disabled = [[AirFloatSharedAppDelegate.settings objectForKey:@"keepScreenLit"] boolValue];
    
    disabled &= (![[AirFloatSharedAppDelegate.settings objectForKey:@"keepScreenLitOnlyWhenReceiving"] boolValue] || (_server != NULL && raop_server_is_recording(_server)));
    
    disabled &= (![[AirFloatSharedAppDelegate.settings objectForKey:@"keepScreenLitOnlyWhenConnectedToPower"] boolValue] || ([UIDevice currentDevice].batteryState == UIDeviceBatteryStateCharging || [UIDevice currentDevice].batteryState == UIDeviceBatteryStateFull));
    
    [UIApplication sharedApplication].idleTimerDisabled = disabled;
    
}

- (void)settingsUpdated:(NSNotification *)notification {
    
    [self updateScreenIdleState];
    
}

- (void)batteryStateChanged:(NSNotification *)notification {
    
    [self updateScreenIdleState];
    
}

@end

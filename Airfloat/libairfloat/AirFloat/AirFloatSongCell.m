//
//  AirFloatSongCell.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatSongCell.h"

@interface AirFloatSongCell (Private)

- (void)_resetFrames;

@end

@implementation AirFloatSongCell

#pragma mark - Class Methods

+ (AirFloatSongCell*)cellForTableView:(UITableView *)tableView {
    
    AirFloatSongCell* cell = (AirFloatSongCell*)[tableView dequeueReusableCellWithIdentifier:@"songCell"];
    
    if (!cell)
        cell = [NSMainBundle objectInNib:@"AirFloatSongCell" ofClass:[AirFloatSongCell class]];
    
    return cell;
    
}

#pragma mark - Allocation / Deallocation

- (void)dealloc {
    
    self.trackNumberLabel = nil;
    self.trackNameLabel = nil;
    self.artistNameLabel = nil;
    self.timeLabel = nil;
    self.albumNameLabel = nil;
    self.nowPlayingIndicatorView = nil;
    self.seperatorView = nil;

    [super dealloc];
    
}

#pragma mark - Public Properties

@synthesize nowPlayingIndicatorView;
@synthesize trackNumberLabel;
@synthesize trackNameLabel;
@synthesize artistNameLabel;
@synthesize timeLabel;
@synthesize albumNameLabel;
@synthesize seperatorView;
@synthesize displayArtist=_displayArtist;
@synthesize displayAlbum=_displayAlbum;

- (void)setDisplayArtist:(BOOL)displayArtist {
    
    _displayArtist = displayArtist;
    
    self.artistNameLabel.hidden = !_displayArtist;
        
    if (!_displayArtist) {
        
        self.nowPlayingIndicatorView.center = [self.superview convertPoint:self.center toView:self];
        
        CGRect nowPlayingIndicatorFrame = self.nowPlayingIndicatorView.frame;
        CGRect trackNumberFrame = self.trackNumberLabel.frame;
        CGRect trackNameFrame = self.trackNameLabel.frame;
        CGRect timeFrame = self.timeLabel.frame;
        
        nowPlayingIndicatorFrame.origin.x = 10 - floor(nowPlayingIndicatorFrame.size.width / 2);
        trackNumberFrame.origin.y = trackNameFrame.origin.y = timeFrame.origin.y = 0;
        trackNumberFrame.origin.x += 20 - nowPlayingIndicatorFrame.size.width;
        trackNumberFrame.size.height = trackNameFrame.size.height = timeFrame.size.height = 44;
        trackNameFrame.size.width -= timeFrame.size.width + 20 - nowPlayingIndicatorFrame.size.width;
        trackNameFrame.origin.x += 20 - nowPlayingIndicatorFrame.size.width;
        
        self.nowPlayingIndicatorView.frame = nowPlayingIndicatorFrame;
        self.trackNumberLabel.frame = trackNumberFrame;
        self.trackNameLabel.frame = trackNameFrame;
        self.timeLabel.frame = timeFrame;        
        
    }
    
}

- (void)setDisplayAlbum:(BOOL)displayAlbum {
    
    [self _resetFrames];
    
    if (displayAlbum)
        self.displayArtist = YES;
    else {
        CGRect frame = self.frame;
        frame.size.height = 44;
        self.frame = frame;        
    }
    
    _displayAlbum = displayAlbum;
    
    self.albumNameLabel.hidden = !_displayAlbum;
        
}

- (NSString*)reuseIdentifier {
    
    return @"songCell";
    
}

#pragma mark - Private Methods

- (void)_resetFrames {
    
    self.frame = CGRectMake(0, self.frame.origin.y, 320, 57);
    
    self.nowPlayingIndicatorView.frame = _nowPlayIndicatorFrame;
    self.trackNumberLabel.frame = _trackNumberFrame;
    self.trackNameLabel.frame = _trackNameFrame;
    self.artistNameLabel.frame = _artistNameFrame;
    self.timeLabel.frame = _timeFrame;
    self.albumNameLabel.frame = _albumNameFrame;
    self.seperatorView.hidden = NO;
    
    _displayArtist = _displayAlbum = NO;
    
}

#pragma mark - Public Methods

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    if ([UICurrentDevice.systemVersion doubleValue] < 5) {
        self.artistNameLabel.font = self.timeLabel.font = [UIFont fontWithName:@"HelveticaNeue" size:12];
        self.albumNameLabel.font = [UIFont fontWithName:@"HelveticaNeue-Italic" size:12];
    }
    
    _displayArtist = _displayAlbum = YES;
    
    _nowPlayIndicatorFrame = self.nowPlayingIndicatorView.frame;
    _trackNumberFrame = self.trackNumberLabel.frame;
    _trackNameFrame = self.trackNameLabel.frame;
    _artistNameFrame = self.artistNameLabel.frame;
    _timeFrame = self.timeLabel.frame;
    _albumNameFrame = self.albumNameLabel.frame;
    
    CGRect coloredBackgroundViewFrame = self.selectedBackgroundView.bounds;
    coloredBackgroundViewFrame.size.height -= 1;
    
    UIView* coloredBackgroundView = [UIView viewWithFrame:coloredBackgroundViewFrame];
    coloredBackgroundView.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.3];
    coloredBackgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    coloredBackgroundView.hidden = YES;
        
    [self insertSubview:coloredBackgroundView atIndex:0];
    
}

- (void)prepareForReuse {
    
    [self _resetFrames];
    
    self.trackNumberLabel.text = self.trackNameLabel.text = self.artistNameLabel.text = self.timeLabel.text = self.albumNameLabel.text = @"";
    
    self.albumNameLabel.hidden = NO;
    self.nowPlayingIndicatorView.hidden = YES;
        
    ((UIView*)[self.subviews objectAtIndex:0]).hidden = YES;
    
}

- (void)setHighlighted:(BOOL)highlighted animated:(BOOL)animated {
    
    ((UIView*)[self.subviews objectAtIndex:0]).hidden = !highlighted;
    
}

- (void)adjustTrackNumberToWidth:(CGFloat)width {
    
    width += 5;
    CGFloat diff = width - self.trackNumberLabel.frame.size.width;
    
    self.trackNumberLabel.width = width;
    self.trackNameLabel.x += diff;
    self.trackNameLabel.width -= diff;
    self.artistNameLabel.x += diff;
    self.artistNameLabel.width -= diff;
    self.albumNameLabel.x += diff;
    self.albumNameLabel.width -= diff;

    if (_displayArtist)
        self.nowPlayingIndicatorView.x += diff;
    
}

@end

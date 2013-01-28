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

- (void)_setHighlight:(BOOL)animated;
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

+ (CGSize)sizeForStyle:(AirFloatSongCellStyle)style {
    
    return CGSizeMake(320, (style == kAirFloatSongCellStyleFull ? 57 : 44));
    
}

#pragma mark - Allocation / Deallocation

- (void)dealloc {
    
    [_coloredBackgroundView release];
    
    self.trackNumberLabel = nil;
    self.trackNameLabel = nil;
    self.artistNameLabel = nil;
    self.timeLabel = nil;
    self.albumNameLabel = nil;
    self.nowPlayingIndicatorView = nil;
    self.seperatorView = nil;
    self.numberContentView = nil;
    self.infoContentView = nil;
    self.infoView = nil;

    [super dealloc];
    
}

#pragma mark - Public Properties

@synthesize infoView;
@synthesize numberContentView;
@synthesize infoContentView;
@synthesize nowPlayingIndicatorView;
@synthesize trackNumberLabel;
@synthesize trackNameLabel;
@synthesize artistNameLabel;
@synthesize timeLabel;
@synthesize albumNameLabel;
@synthesize seperatorView;
@synthesize style=_style;

- (void)setStyle:(AirFloatSongCellStyle)style {
    
    [self willChangeValueForKey:@"style"];
    
    [self _resetFrames];
    
    self.artistNameLabel.hidden = self.albumNameLabel.hidden = NO;
    
    CGFloat cellHeight = [[self class] sizeForStyle:style].height;
    
    self.infoView.height = self.numberContentView.height = self.infoContentView.height = cellHeight;

    _style = style;
    if (_style == kAirFloatSongCellStyleSimple) {
        
        
        self.artistNameLabel.hidden = self.albumNameLabel.hidden = YES;
        self.trackNameLabel.frame = CGRectMake(0, 0, self.contentView.width - self.trackNumberLabel.width, 44);
        self.timeLabel.frame = CGRectMake(self.timeLabel.x, 0, self.timeLabel.width, 44);
        
        self.trackNumberLabel.frame = CGRectMake(0, 0, _trackWidth, 44);
        self.nowPlayingIndicatorView.frame = CGRectMake(5, floor((44 - self.nowPlayingIndicatorView.height) / 2), self.nowPlayingIndicatorView.width, self.nowPlayingIndicatorView.height);
        
        self.nowPlayingIndicatorView.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin;
        
    }
    
    [self adjustTrackNumberToWidth:_trackWidth];
    
    [self didChangeValueForKey:@"style"];
    
}

- (NSString*)reuseIdentifier {
    
    return @"songCell";
    
}

#pragma mark - Private Methods

- (void)_resetFrames {
    
    self.numberContentView.frame = _numberContentFrame;
    self.infoContentView.frame = _infoContentFrame;
    
    self.nowPlayingIndicatorView.frame = _nowPlayIndicatorFrame;
    self.trackNumberLabel.frame = _trackNumberFrame;
    self.trackNameLabel.frame = _trackNameFrame;
    self.artistNameLabel.frame = _artistNameFrame;
    self.timeLabel.frame = _timeFrame;
    self.albumNameLabel.frame = _albumNameFrame;
    
    self.infoContentView.width -= self.width - self.infoView.width;
    
    self.seperatorView.hidden = NO;
    
    self.infoContentView.width -= self.bounds.size.width - self.contentView.bounds.size.width;
    
    self.nowPlayingIndicatorView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
    
    _style = kAirFloatSongCellStyleFull;
    
}

- (void)_setHighlight:(BOOL)animated {
    
    BOOL highlighted = (_selected || _highlighted);
    
    UIView* selectionView = [self.subviews objectAtIndex:0];
    
    if (selectionView.hidden == highlighted) {
        
        if (animated) {
            
            if (highlighted) {
                selectionView.alpha = 0.0;
                selectionView.hidden = NO;
            }
            
            [UIView animateWithDuration:1.0
                                  delay:0.0
                                options:UIViewAnimationCurveEaseOut
                             animations:^{
                                 selectionView.alpha = (highlighted ? 1.0 : 0.0);
                             } completion:^(BOOL finished) {
                                 if (!highlighted) {
                                     selectionView.hidden = YES;
                                     selectionView.alpha = 1.0;
                                 }
                             }];
            
        } else {
            selectionView.hidden = !highlighted;
            selectionView.alpha = 1.0;
        }
        
    }
    
}

#pragma mark - Public Methods

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    if ([UICurrentDevice.systemVersion doubleValue] < 5) {
        self.artistNameLabel.font = self.timeLabel.font = [UIFont fontWithName:@"HelveticaNeue" size:12];
        self.albumNameLabel.font = [UIFont fontWithName:@"HelveticaNeue-Italic" size:12];
    }
    
    _numberContentFrame = self.numberContentView.frame;
    _infoContentFrame = self.infoContentView.frame;
    
    _nowPlayIndicatorFrame = self.nowPlayingIndicatorView.frame;
    _trackNumberFrame = self.trackNumberLabel.frame;
    _trackNameFrame = self.trackNameLabel.frame;
    _artistNameFrame = self.artistNameLabel.frame;
    _timeFrame = self.timeLabel.frame;
    _albumNameFrame = self.albumNameLabel.frame;
    
    _trackWidth = self.numberContentView.width;
    
    CGRect coloredBackgroundViewFrame = self.selectedBackgroundView.bounds;
    coloredBackgroundViewFrame.size.height -= 1;
    
    _coloredBackgroundView = [[UIView alloc] initWithFrame:coloredBackgroundViewFrame];
    _coloredBackgroundView.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.2];
    _coloredBackgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    _coloredBackgroundView.hidden = YES;
        
    [self insertSubview:_coloredBackgroundView atIndex:0];
    
}

- (void)prepareForReuse {
    
    [self _resetFrames];
    
    self.trackNumberLabel.text = self.trackNameLabel.text = self.artistNameLabel.text = self.timeLabel.text = self.albumNameLabel.text = @"";
    
    self.artistNameLabel.hidden = self.albumNameLabel.hidden = NO;
    
    self.nowPlayingIndicatorView.hidden = _coloredBackgroundView.hidden = YES;
    
    self.infoView.frame = self.bounds;
    
}

- (void)setHighlighted:(BOOL)highlighted animated:(BOOL)animated {
    
    _highlighted = highlighted;
    [self _setHighlight:animated];
    
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    
    _selected = selected;
    [self _setHighlight:animated];
    
}

- (void)adjustTrackNumberToWidth:(CGFloat)width {
    
    _trackWidth = width;
    
    if (self.style == kAirFloatSongCellStyleSimple)
        width += self.nowPlayingIndicatorView.width + 5;
    
    self.numberContentView.width = width + 5;
    self.trackNumberLabel.width = width + 5;
    self.infoContentView.x = self.numberContentView.width;
    self.infoContentView.width = self.infoView.width - self.numberContentView.width;
    
    self.trackNameLabel.x = self.artistNameLabel.x = self.albumNameLabel.x = 7;
    self.trackNameLabel.width = self.albumNameLabel.width = self.infoContentView.width - (_infoContentFrame.size.width - _trackNameFrame.size.width);
    self.albumNameLabel.width = self.infoContentView.width - (_infoContentFrame.size.width - _albumNameFrame.size.width);
    
    if (_style == kAirFloatSongCellStyleSimple)
        self.trackNameLabel.width = self.infoContentView.width - (_infoContentFrame.size.width - _albumNameFrame.size.width);
    
}

@end

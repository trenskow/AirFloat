//
//  AirFloatSongCell.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AirFloatSongCell : UITableViewCell {
    
    CGRect _nowPlayIndicatorFrame;
    CGRect _trackNumberFrame;
    CGRect _trackNameFrame;
    CGRect _artistNameFrame;
    CGRect _timeFrame;
    CGRect _albumNameFrame;
    
    BOOL _displayArtist;
    BOOL _displayAlbum;
    
}

+ (AirFloatSongCell*)cellForTableView:(UITableView*)tableView;

@property (retain, nonatomic) IBOutlet UIView *nowPlayingIndicatorView;
@property (retain, nonatomic) IBOutlet UILabel *trackNumberLabel;
@property (retain, nonatomic) IBOutlet UILabel *trackNameLabel;
@property (retain, nonatomic) IBOutlet UILabel *artistNameLabel;
@property (retain, nonatomic) IBOutlet UILabel *timeLabel;
@property (retain, nonatomic) IBOutlet UILabel *albumNameLabel;
@property (retain, nonatomic) IBOutlet UIView *seperatorView;

@property (assign, nonatomic,getter=shouldDisplayArtist) BOOL displayArtist;
@property (assign, nonatomic,getter=shouldDisplayAlbum) BOOL displayAlbum;

- (void)adjustTrackNumberToWidth:(CGFloat)width;

@end

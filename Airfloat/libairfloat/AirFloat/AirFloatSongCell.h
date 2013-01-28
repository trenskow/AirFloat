//
//  AirFloatSongCell.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

typedef enum {
    
    kAirFloatSongCellStyleFull = 0,
    kAirFloatSongCellStyleSimple = 1
    
} AirFloatSongCellStyle;

@interface AirFloatSongCell : UITableViewCell {
    
    UIView* _coloredBackgroundView;
    
    CGRect _numberContentFrame;
    CGRect _infoContentFrame;
    
    CGRect _nowPlayIndicatorFrame;
    CGRect _trackNumberFrame;
    CGRect _trackNameFrame;
    CGRect _artistNameFrame;
    CGRect _timeFrame;
    CGRect _albumNameFrame;
    
    AirFloatSongCellStyle _style;
    
    CGFloat _trackWidth;
    
    BOOL _selected;
    BOOL _highlighted;
    
}

+ (AirFloatSongCell*)cellForTableView:(UITableView*)tableView;
+ (CGSize)sizeForStyle:(AirFloatSongCellStyle)style;

@property (retain, nonatomic) IBOutlet UIView *infoView;
@property (retain, nonatomic) IBOutlet UIView *numberContentView;
@property (retain, nonatomic) IBOutlet UIView *infoContentView;

@property (retain, nonatomic) IBOutlet UIView *nowPlayingIndicatorView;
@property (retain, nonatomic) IBOutlet UILabel *trackNumberLabel;
@property (retain, nonatomic) IBOutlet UILabel *trackNameLabel;
@property (retain, nonatomic) IBOutlet UILabel *artistNameLabel;
@property (retain, nonatomic) IBOutlet UILabel *timeLabel;
@property (retain, nonatomic) IBOutlet UILabel *albumNameLabel;
@property (retain, nonatomic) IBOutlet UIView *seperatorView;

@property (assign, nonatomic) AirFloatSongCellStyle style;

- (void)adjustTrackNumberToWidth:(CGFloat)width;

@end

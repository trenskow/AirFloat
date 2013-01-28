//
//  AirPlayScrollView.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@class AirFloatTableView;

@protocol AirFloatTableViewDelegate

@optional
- (UIImage*)airFloatTableView:(AirFloatTableView*)tableView imageForPageIndicator:(NSInteger)page;
- (CGPoint)airFloatTableView:(AirFloatTableView*)tableView offsetForPage:(NSInteger)page withSuggestedOffset:(CGPoint)offset;

@end

@interface AirFloatTableView : UITableView {
    
    BOOL _showsShortCutScrollBar;
    UIColor* _shortCutScrollBarColor;
    
    UIView* _shortCutBarView;
    
    BOOL _isAnimatingShortCutBarAppearence;
    
    CGFloat _shortCutScrollBarWidth;
    
}

@property (nonatomic,assign) BOOL showsShortCutScrollBar;
@property (nonatomic,retain) UIColor* shortCutScrollBarColor;
@property (nonatomic,assign) CGFloat shortCutScrollBarWidth;
@property (nonatomic,readonly) NSInteger pageCount;

- (void)setShowsShortCutScrollBar:(BOOL)showsShortCutScrollBar animated:(BOOL)animated;

- (void)reloadShortCutScrollBar;
- (NSInteger)pageForRowAtIndexPath:(NSIndexPath*)indexPath;

@end

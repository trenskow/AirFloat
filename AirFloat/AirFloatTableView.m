//
//  AirPlayScrollView.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "AirFloatAdditions.h"
#import "AirFloatTapPanGestureRecognizer.h"
#import "AirFloatMacros.h"
#import "AirFloatTableView.h"

@interface AirFloatTableView (Private)

- (BOOL)_delegateRespondsToSelector:(SEL)selector;
- (void)_setupShortCutScrollBar;
- (void)_removeShortCutScrollBar;
- (void)_tapGestureRecognized:(AirFloatTapPanGestureRecognizer*)tapGestureRecognizer;

@end

@implementation AirFloatTableView

#pragma mark - Allocation / Deallocation

- (void)dealloc {
    
    [self _removeShortCutScrollBar];
    
    [super dealloc];
    
}

#pragma mark - Private Methods

- (BOOL)_delegateRespondsToSelector:(SEL)selector {
    
    return (self.delegate && [(id)self.delegate conformsToProtocol:@protocol(AirFloatTableViewDelegate)] && [(id)self.delegate respondsToSelector:selector]);
    
}

- (void)_setupShortCutScrollBar {
    
    if (_shortCutBarView)
        [self _removeShortCutScrollBar];
    
    CGFloat viewWidth = self.shortCutScrollBarWidth;
    CGRect viewFrame = CGRectMake(self.bounds.size.width - (viewWidth * 1.5),
                                  self.contentOffset.y + (viewWidth / 2),
                                  viewWidth,
                                  self.bounds.size.height - viewWidth);
    
    _shortCutBarView = [[UIView alloc] initWithFrame:viewFrame];
    
    UIColor* color = [self.shortCutScrollBarColor colorWithAlphaComponent:0.6];
    
    _shortCutBarView.layer.masksToBounds = YES;
    _shortCutBarView.layer.cornerRadius = floor(viewWidth / 2);
    _shortCutBarView.layer.shouldRasterize = YES;
    _shortCutBarView.layer.rasterizationScale = self.window.screen.scale;
    
    NSInteger pageCount = self.pageCount;
    
    for (CGFloat page = 0 ; page < pageCount ; page++) {
        
        CALayer* pageIndicatorLayer = [[CALayer alloc] init];
        CGSize pageIndicatorSize = CGSizeZero;
        
        UIImage* pageIndicatorImage = nil;
        
        if ([self _delegateRespondsToSelector:@selector(airFloatTableView:imageForPageIndicator:)] && (pageIndicatorImage = [(id<AirFloatTableViewDelegate>)self.delegate airFloatTableView:self imageForPageIndicator:(NSInteger)page])) {
            
            UIImage* image = [[UIImage imageWithSolidColor:[self.shortCutScrollBarColor colorWithAlphaComponent:0.6] withSize:pageIndicatorImage.size andScale:pageIndicatorImage.scale] imageByApplyingMask:pageIndicatorImage];
            
            pageIndicatorSize = image.size;
            pageIndicatorLayer.contents = (id)image.CGImage;
            
        } else {
            
            pageIndicatorSize = (page == 0 || page == pageCount - 1 ? CGSizeMake(5, 5) : CGSizeMake(3, 3));
            
            pageIndicatorLayer.backgroundColor = color.CGColor;
            pageIndicatorLayer.masksToBounds = YES;
            pageIndicatorLayer.cornerRadius = pageIndicatorSize.width / 2;
            
        }
        
        CGFloat y = (((viewFrame.size.height - viewFrame.size.width) / (pageCount - 1)) * page) + (viewFrame.size.width / 2);
        
        pageIndicatorLayer.frame = CGRectMake((viewFrame.size.width - pageIndicatorSize.width) / 2, y - (pageIndicatorSize.height / 2), pageIndicatorSize.width, pageIndicatorSize.height);
        
        pageIndicatorLayer.shouldRasterize = YES;
        pageIndicatorLayer.rasterizationScale = self.window.screen.scale;
        
        [_shortCutBarView.layer addSublayer:pageIndicatorLayer];
        
        [pageIndicatorImage release];
        
    }
    
    AirFloatTapPanGestureRecognizer* tapGestureRecognizer = [[AirFloatTapPanGestureRecognizer alloc] initWithTarget:self action:@selector(_tapGestureRecognized:)];
    [_shortCutBarView addGestureRecognizer:tapGestureRecognizer];
    [tapGestureRecognizer release];
    
    self.scrollIndicatorInsets = UIEdgeInsetsMake(viewWidth - 5, 0, viewWidth - 5, 2);
    
    [self addSubview:_shortCutBarView];
    
}

- (void)_removeShortCutScrollBar {
    
    [_shortCutBarView removeFromSuperview];
    [_shortCutBarView release];
    _shortCutBarView = nil;
    
}

#pragma mark - Private Actions

- (void)_tapGestureRecognized:(AirFloatTapPanGestureRecognizer *)tapGestureRecognizer {
    
    switch (tapGestureRecognizer.state) {
        case UIGestureRecognizerStateBegan:
            _shortCutBarView.layer.backgroundColor = [self.shortCutScrollBarColor colorWithAlphaComponent:0.6].CGColor;
        case UIGestureRecognizerStateChanged: {
            
            NSInteger pageCount = self.pageCount;
            
            CGFloat pageHeight = (self.contentSize.height - self.bounds.size.height + self.contentInset.top + self.contentInset.bottom) / (CGFloat)pageCount;
            CGFloat pos = ([tapGestureRecognizer locationInView:_shortCutBarView].y - (_shortCutBarView.frame.size.width / 2)) / (_shortCutBarView.frame.size.height - _shortCutBarView.frame.size.width);
            
            NSInteger page = round(pageCount * MAX(MIN(pos, 1.0), 0.0));
            CGFloat offsetY = page * pageHeight;
            
            CGPoint offset = CGPointMake(0, offsetY - self.contentInset.top);
            
            if ([self _delegateRespondsToSelector:@selector(airFloatTableView:offsetForPage:withSuggestedOffset:)])
                offset = [(id<AirFloatTableViewDelegate>)self.delegate airFloatTableView:self offsetForPage:page withSuggestedOffset:offset];
            
            self.contentOffset = offset;
            
            break;
        }
        case UIGestureRecognizerStateEnded:
            _shortCutBarView.layer.backgroundColor = nil;
            break;
        default:
            break;
            
    }
    
}

#pragma mark - Public Properties

@synthesize showsShortCutScrollBar=_showsShortCutScrollBar;
@synthesize shortCutScrollBarColor=_shortCutScrollBarColor;
@synthesize shortCutScrollBarWidth=_shortCutScrollBarWidth;

- (void)setShowsShortCutScrollBar:(BOOL)showsShortCutScrollBar {
    
    [self willChangeValueForKey:@"showsShortCutScrollBar"];
    
    _showsShortCutScrollBar = showsShortCutScrollBar;
    
    [self reloadShortCutScrollBar];
    
    [self didChangeValueForKey:@"showsShortCutScrollBar"];
    
}

- (UIColor*)shortCutScrollBarColor {
    
    if (!_shortCutScrollBarColor)
        return [UIColor blackColor];
    
    return _shortCutScrollBarColor;
    
}

- (CGFloat)shortCutScrollBarWidth {
    
    if (!_shortCutScrollBarWidth)
        return 18;
    
    return _shortCutScrollBarWidth;
    
}

- (void)setContentSize:(CGSize)contentSize {
    
    [super setContentSize:contentSize];
    
    [self reloadShortCutScrollBar];
    
}

- (NSInteger)pageCount {
    
    if (_shortCutBarView) {
        CGFloat maxCount = (_shortCutBarView.frame.size.height - _shortCutBarView.frame.size.width) / 10;
        return MIN(self.contentSize.height / self.bounds.size.height, maxCount);
    }
    
    return 0;
    
}

- (void)setContentOffset:(CGPoint)contentOffset {
    
    [super setContentOffset:contentOffset];
    
    if ((self.showsShortCutScrollBar && _shortCutBarView) || _isAnimatingShortCutBarAppearence) {
        CGRect shortCutBarViewFrame = _shortCutBarView.frame;
        shortCutBarViewFrame.origin.y = self.contentOffset.y + (_shortCutBarView.frame.size.width / 2);
        _shortCutBarView.frame = shortCutBarViewFrame;
    }
    
}

- (void)setFrame:(CGRect)frame {
    
    [super setFrame:frame];    
    
    [self reloadShortCutScrollBar];
    
}

#pragma mark - Public Methods

- (void)reloadShortCutScrollBar {
    
    if (!_isAnimatingShortCutBarAppearence) {
        
        [self _removeShortCutScrollBar];
        if (self.showsShortCutScrollBar)
            [self _setupShortCutScrollBar];
        
    }
    
}

- (void)reloadData {
    
    [super reloadData];
    
    [self reloadShortCutScrollBar];
    
}

- (void)setShowsShortCutScrollBar:(BOOL)showsShortCutScrollBar animated:(BOOL)animated {
    
    if (self.showsShortCutScrollBar != showsShortCutScrollBar) {
        
        if (animated) {
            
            _isAnimatingShortCutBarAppearence = YES;
            
            if (showsShortCutScrollBar && !_shortCutBarView) {
                [self _setupShortCutScrollBar];
                _shortCutBarView.alpha = 0.0;
            }
            
            [UIView animateWithDuration:0.3
                                  delay:0.0
                                options:UIViewAnimationCurveEaseOut
                             animations:^{
                                 _shortCutBarView.alpha = (showsShortCutScrollBar ? 1.0 : 0.0);
                             } completion:^(BOOL finished) {
                                 
                                 _isAnimatingShortCutBarAppearence = NO;
                                 
                                 [self setShowsShortCutScrollBar:showsShortCutScrollBar];
                                 
                             }];
            
        } else
            [self setShowsShortCutScrollBar:showsShortCutScrollBar];

    }
    
}

- (NSArray*)indexPathsForPage:(NSInteger)page {
    
    if (_shortCutBarView) {
        
        CGFloat pageHeight = (self.contentSize.height + self.contentInset.top + self.contentInset.bottom) / (CGFloat)self.pageCount;
        
        NSArray* indexPaths = [self indexPathsForRowsInRect:CGRectMake(0, pageHeight * page, self.bounds.size.width, pageHeight)];
        NSArray* nextIndexPaths = [self indexPathsForRowsInRect:CGRectMake(0, pageHeight * (page + 1), self.bounds.size.width, self.rowHeight)];
        
        if ([nextIndexPaths count] > 0 && [indexPaths count] > 0 && [[nextIndexPaths objectAtIndex:0] compare:[indexPaths lastObject]] == NSOrderedSame)
            indexPaths = [indexPaths subarrayWithRange:NSMakeRange(0, [indexPaths count] - 1)];
        
        return indexPaths;
        
    }
    
    return nil;
    
}

- (NSInteger)pageForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    CGFloat pageHeight = (self.contentSize.height + self.contentInset.top + self.contentInset.bottom) / (CGFloat)self.pageCount;
    
    return floor([self rectForRowAtIndexPath:indexPath].origin.y / pageHeight);
    
}

@end

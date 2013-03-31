//
//  AirFloatScrollingLabel.m
//  AirFloat
//
//  Created by Kristian Trenskow on 12/30/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "AirFloatScrollingLabel.h"

@interface AirFloatScrollingLabel () {
    
    CAGradientLayer* _maskLayer;
    NSMutableArray* _currentLabels;
    
}

@end

@implementation AirFloatScrollingLabel

#pragma mark - Allocation / Deallocation

- (void)dealloc {
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    
    [_maskLayer release];
    [_currentLabels release];
    
    [super dealloc];
    
}

#pragma mark - Private Methods

- (void)_scrollLabel:(UILabel *)label {
    
    if (!label.superview)
        return;
    
    CGFloat widthDiff = label.frame.size.width - self.bounds.size.width;
    
    if (widthDiff > 0.0) {
        
        __block CGRect endFrame = label.frame;
        endFrame.origin.x = -widthDiff;
        
        [UIView animateWithDuration:widthDiff / 25.0
                              delay:0.3
                            options:UIViewAnimationOptionCurveLinear
                         animations:^{
                             label.frame = endFrame;
                         } completion:^(BOOL finished) {
                             if (label.superview) {
                                 [UIView animateWithDuration:0.3
                                                       delay:0.0
                                                     options:UIViewAnimationOptionCurveEaseIn
                                                  animations:^{
                                                      _maskLayer.colors = @[(id)[UIColor clearColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor];
                                                  } completion:nil];
                                 [UIView animateWithDuration:0.5
                                                       delay:3.0
                                                     options:UIViewAnimationOptionCurveEaseIn
                                                  animations:^{
                                                      label.alpha = 0.0;
                                                  } completion:^(BOOL finished) {
                                                      if (label.superview) {
                                                          endFrame.origin.x = 0;
                                                          label.frame = endFrame;
                                                          _maskLayer.colors = @[(id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor clearColor].CGColor];
                                                          [UIView animateWithDuration:0.5
                                                                                delay:0.0
                                                                              options:UIViewAnimationOptionCurveEaseOut
                                                                           animations:^{
                                                                               label.alpha = 1.0;
                                                                           } completion:^(BOOL finished) {
                                                                               [self performSelector:@selector(_scrollLabel:) withObject:label afterDelay:10];
                                                                           }];
                                                      }
                                                  }];
                             }
                         }];
        
        [UIView animateWithDuration:0.3
                         animations:^{
                             _maskLayer.colors = @[(id)[UIColor clearColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor clearColor].CGColor];
                         }];
        
    }
    
}

#pragma mark - Public Methods

- (NSString *)text {
    
    return [[_currentLabels lastObject] text];
    
}

- (void)setText:(NSString *)text {
    
    if (!_maskLayer) {
        
        _maskLayer = [[CAGradientLayer layer] retain];
        _maskLayer.frame = self.bounds;
        
        _maskLayer.colors = @[(id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor];
        _maskLayer.startPoint = CGPointMake(0.0, 0.5);
        _maskLayer.endPoint = CGPointMake(1.0, 0.5);
        _maskLayer.locations = @[@0.0, @(0.017), @(0.983), @1.0];
        
        self.layer.mask = _maskLayer;
        
    }
    
    CGRect frame = self.bounds;
    UILabel* newLabel = [[UILabel alloc] initWithFrame:frame];
    
    newLabel.font = self.font;
    newLabel.textColor = self.textColor;
    newLabel.shadowColor = self.shadowColor;
    newLabel.shadowOffset = self.shadowOffset;
    newLabel.text = text;
    newLabel.backgroundColor = [UIColor clearColor];
    newLabel.opaque = NO;
    newLabel.textAlignment = self.textAlignment;
    
    super.text = nil;
    
    [newLabel sizeToFit];
    frame.size.width = MAX(newLabel.bounds.size.width, self.bounds.size.width);
    newLabel.frame = frame;
    
    if (!_currentLabels)
        _currentLabels = [[NSMutableArray alloc] init];
    
    while ([_currentLabels count] > 0) {
        [[_currentLabels lastObject] removeFromSuperview];
        [_currentLabels removeLastObject];
    }
    
    if (newLabel.frame.size.width > self.bounds.size.width)
        _maskLayer.colors = @[(id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor clearColor].CGColor];
    else
        _maskLayer.colors = @[(id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor];
    
    [_currentLabels addObject:newLabel];
    
    [self addSubview:newLabel];
    
    [newLabel release];
    
    [self performSelector:@selector(_scrollLabel:) withObject:newLabel afterDelay:2.7];
        
}

@end

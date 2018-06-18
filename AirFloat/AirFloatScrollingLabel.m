//
//  AirFloatScrollingLabel.m
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
    
}

#pragma mark - Private Methods

- (void)_scrollLabel:(UILabel *)label {
    
    if (!label.superview)
        return;
    
    CGFloat widthDiff = label.frame.size.width - self.bounds.size.width;
    
    if (widthDiff > 0.0) {
        
        __block CGRect endFrame = label.frame;
        endFrame.origin.x = -widthDiff;
        
        label.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleRightMargin;
        
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
                                                      self->_maskLayer.colors = @[(id)[UIColor clearColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor];
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
                                                          self->_maskLayer.colors = @[(id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor clearColor].CGColor];
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
                             self->_maskLayer.colors = @[(id)[UIColor clearColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor blackColor].CGColor, (id)[UIColor clearColor].CGColor];
                         }];
        
    }
    
}

#pragma mark - Public Methods

- (NSString *)text {
    
    return [[_currentLabels lastObject] text];
    
}

- (void)setText:(NSString *)text {
    
    if (!_maskLayer) {
        
        _maskLayer = [CAGradientLayer layer];
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
    newLabel.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    
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
    
    [self performSelector:@selector(_scrollLabel:) withObject:newLabel afterDelay:2.7];
        
}

@end

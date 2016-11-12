//
//  AirFloatSwitch.m
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

#import "AirFloatSwitch.h"

@interface AirFloatSwitch () {
    
    UIImageView* _backgroundImageView;
    UIImageView* _backgroundHighligthedImageView;
    
    UIView* _knobView;
    UIImageView* _knobImageView;
    UIImageView* _knobOffImageView;
    UIImageView* _knobOnImageView;
    
    UIPanGestureRecognizer* _knobPanGestureRecognizer;
    UITapGestureRecognizer* _tapGestureRecoginzer;
    
    UILabel* _enabledLabel;
    UILabel* _disabledLabel;
    
    UIView* _enabledLabelContainer;
    UIView* _disabledLabelContainer;
    
    BOOL _on;
    
}

@end

@implementation AirFloatSwitch

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    _backgroundImageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"SettingsSwitchBackground.png"]];
    _backgroundHighligthedImageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"SettingsSwitchBackgroundHighlighted.png"]];
    
    _backgroundImageView.contentMode = UIViewContentModeTopRight;
    _backgroundHighligthedImageView.contentMode = UIViewContentModeTopLeft;
    
    _backgroundImageView.clipsToBounds = _backgroundHighligthedImageView.clipsToBounds = YES;
    
    [self addSubview:_backgroundImageView];
    [self addSubview:_backgroundHighligthedImageView];
    
    _enabledLabelContainer = [[UIView alloc] initWithFrame:CGRectMake(8, 1, 72, 24)];
    _disabledLabelContainer = [[UIView alloc] initWithFrame:CGRectMake(8, 1, 72, 24)];
    _enabledLabelContainer.clipsToBounds = _disabledLabelContainer.clipsToBounds = YES;
    
    [self addSubview:_enabledLabelContainer];
    [self addSubview:_disabledLabelContainer];
    
    _enabledLabel = [[UILabel alloc] initWithFrame:_enabledLabelContainer.bounds];
    _disabledLabel = [[UILabel alloc] initWithFrame:_disabledLabelContainer.bounds];
    
    _enabledLabel.text = @"Enabled";
    _disabledLabel.text = @"Disabled";
    
    _enabledLabel.lineBreakMode = _disabledLabel.lineBreakMode = NSLineBreakByClipping;
    _enabledLabel.textAlignment = NSTextAlignmentLeft;
    _disabledLabel.textAlignment = NSTextAlignmentRight;
    _enabledLabel.autoresizingMask = UIViewAutoresizingFlexibleRightMargin;
    _disabledLabel.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    _enabledLabel.font = [UIFont boldSystemFontOfSize:12];
    _disabledLabel.font = [UIFont systemFontOfSize:12];
    
    _enabledLabel.textColor = _disabledLabel.textColor = [UIColor colorWithWhite:1.0 alpha:0.9];
    _enabledLabel.backgroundColor = _disabledLabel.backgroundColor = [UIColor clearColor];
    _enabledLabel.opaque = _disabledLabel.opaque = NO;
    
    [_enabledLabelContainer addSubview:_enabledLabel];
    [_disabledLabelContainer addSubview:_disabledLabel];
    
    _knobView = [[UIView alloc] initWithFrame:CGRectMake(0, 2, 26, 27)];
    _knobView.backgroundColor = [UIColor clearColor];
    _knobView.opaque = NO;
    
    [self addSubview:_knobView];
    
    _knobImageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"SettingsSwitchKnob.png"]];
    _knobOffImageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"SettingsSwitchKnobOff.png"]];
    _knobOnImageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"SettingsSwitchKnobOn.png"]];
    
    [_knobView addSubview:_knobImageView];
    
    [_knobView addSubview:_knobOffImageView];
    [_knobView addSubview:_knobOnImageView];
    
    _knobPanGestureRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(panGestureRecognized:)];    
    [_knobView addGestureRecognizer:_knobPanGestureRecognizer];
    
    _tapGestureRecoginzer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapGestureRecognized:)];
    [self addGestureRecognizer:_tapGestureRecoginzer];
    
    self.on = NO;
    
}

- (void)panGestureRecognized:(UIPanGestureRecognizer *)panGestureRecognizer {
    
    switch (panGestureRecognizer.state) {
        case UIGestureRecognizerStateChanged:
            [self setKnobPosition:MIN(72, MAX(0, [panGestureRecognizer locationInView:self].x - 8)) / 72.0 animated:NO];
            break;
        case UIGestureRecognizerStateEnded:
            [self setOn:([panGestureRecognizer locationInView:self].x >= 44.0)
               animated:YES];
            break;
        default:
            break;
    }
    
}

- (void)tapGestureRecognized:(UITapGestureRecognizer *)tapGestureRecognizer {
    
    if (tapGestureRecognizer.state == UIGestureRecognizerStateRecognized)
        [self setOn:!self.on animated:YES];
    
}

- (void)setKnobPosition:(CGFloat)x animated:(BOOL)animated {
    
    CGRect enabledLabelFrame = _enabledLabelContainer.frame;
    CGRect disabledLabelFrame = _disabledLabelContainer.frame;
    
    enabledLabelFrame.size.width = 72 * x;
    disabledLabelFrame.origin.x = (72 * x) + 8;
    disabledLabelFrame.size.width = 72 * (1.0 - x);
    
    CGRect backgroundImageFrame = _backgroundImageView.frame;
    CGRect backgroundHighlightedFrame = _backgroundHighligthedImageView.frame;
    
    backgroundHighlightedFrame.size.width = 88 * x;
    backgroundImageFrame.origin.x = 88 * x;
    backgroundImageFrame.size.width = 88 * (1.0 - x);
    
    if (animated) {
        [UIView beginAnimations:nil context:NULL];
        [UIView setAnimationCurve:UIViewAnimationCurveEaseOut];
    }
    
    _knobView.center = CGPointMake(12 + (64 * x), 15.5);
    
    _enabledLabelContainer.frame = enabledLabelFrame;
    _disabledLabelContainer.frame = disabledLabelFrame;
    
    _backgroundImageView.frame = backgroundImageFrame;
    _backgroundHighligthedImageView.frame = backgroundHighlightedFrame;
    
    if (animated)
        [UIView commitAnimations];
    
}

- (BOOL)on {
    
    return _on;
    
}

- (void)setOn:(BOOL)on animated:(BOOL)animated {
    
    [self willChangeValueForKey:@"on"];
    
    _knobOnImageView.hidden = !on;
    _knobOffImageView.hidden = on;
    
    _on = on;
    
    [self setKnobPosition:(on ? 1.0 : 0.0) animated:animated];
    
    [self didChangeValueForKey:@"on"];
    
    [self sendActionsForControlEvents:UIControlEventValueChanged];
    
}

- (void)setOn:(BOOL)on {
    
    [self setOn:on animated:NO];
}

@end

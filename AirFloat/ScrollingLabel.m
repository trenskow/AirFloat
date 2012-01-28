//
//  ScrollingLabel.m
//  AirFloat
//
//  Created by Kristian Trenskow on 1/23/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "ScrollingLabel.h"

@interface ScrollingLabel (Private)

- (void)_updateText;

@end

@implementation ScrollingLabel

- (void)_updateText {
    
    _textOffset += 1.0 / self.window.screen.scale;
    if (_textOffset >= _textSize.width)
        _textOffset = 0;
    
    [self setNeedsDisplay];
    
    [self performSelector:@selector(_updateText) withObject:nil afterDelay:(_textOffset > 0 ? 0.1 / self.window.screen.scale : 3.0)];
    
}

- (void)setText:(NSString *)text {
    
    [super setText:text];
    
    _textSize = [self.text sizeWithFont:self.font constrainedToSize:CGSizeMake(CGFLOAT_MAX, self.bounds.size.height) lineBreakMode:UILineBreakModeClip];
    _textOffset = 0;
    
    self.clipsToBounds = YES;
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(_updateText) object:nil];
    
    if (_textSize.width > self.bounds.size.width)
        [self performSelector:@selector(_updateText) withObject:nil afterDelay:3.0];
    
}

- (void)drawRect:(CGRect)rect
{
    
    if (self.text == nil || [self.text length] == 0)
        return;
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextSaveGState(context);
    
    CGContextSetFillColorWithColor(context, self.textColor.CGColor);
    
    CGRect bounds = self.bounds;
    bounds.origin.x -= _textOffset;
    bounds.size.width += _textOffset + 50;
    
    [self.text drawInRect:bounds withFont:self.font lineBreakMode:self.lineBreakMode];
    
    CGContextRestoreGState(context);
    
    
}

@end

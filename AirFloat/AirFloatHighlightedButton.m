//
//  AirFloatHighlightedButton.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/20/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatHighlightedButton.h"

@implementation AirFloatHighlightedButton

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame])) {
        [self awakeFromNib];
        [self setFrame:frame];
    }
    
    return self;
    
}

- (void)dealloc {
    
    [_highlightedView release];
    
    [super dealloc];
    
}

- (void)awakeFromNib {
    
    _highlightedView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"ButtonHighlight.png"]];
    _highlightedView.hidden = YES;
    
    [self addSubview:_highlightedView];
    
}

- (void)setHighlighted:(BOOL)highlighted {
    
    if (self.superview)
        _highlightedView.center = [self.superview convertPoint:self.center toView:self];

    _highlightedView.hidden = !highlighted;
    
    [super setHighlighted:highlighted];
    
}

- (void)setImage:(UIImage *)image forState:(UIControlState)state {
    
    if (state == UIControlStateNormal)
        [self setImage:image forState:UIControlStateHighlighted];
    
    [super setImage:image forState:state];
    
}

@end

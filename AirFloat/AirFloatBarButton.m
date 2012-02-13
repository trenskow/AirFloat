//
//  AirFloatBarButton.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/13/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatBarButton.h"

@implementation AirFloatBarButton

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame]))
        [self awakeFromNib];
    
    return self;
    
}

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    [self setBackgroundImage:[[UIImage imageNamed:@"BarButton.png"] stretchableImageWithLeftCapWidth:5 topCapHeight:0] forState:UIControlStateNormal];
    [self setBackgroundImage:[[UIImage imageNamed:@"BarButtonHighlighted.png"] stretchableImageWithLeftCapWidth:5 topCapHeight:0] forState:UIControlStateNormal];
    
}

@end

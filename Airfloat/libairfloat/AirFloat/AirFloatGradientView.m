//
//  UIGradientView.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "AirFloatGradientView.h"

@implementation AirFloatGradientView

+ (Class)layerClass {
    
    return [CAGradientLayer class];
    
}

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame]))
        [self awakeFromNib];
    
    return self;
    
}

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    NSArray* colors = [self backgroundGradientColors];
    CAGradientLayer* layer = (CAGradientLayer*)self.layer;
    
    layer.locations = [self backgroundGradientColorLocations];
    
    NSMutableArray* cgColors = [NSMutableArray arrayWithCapacity:[colors count]];
    for (UIColor* color in colors)
        [cgColors addObject:(id)color.CGColor];
    
    layer.colors = cgColors;
    
}

- (NSArray*)backgroundGradientColors {
    
    return [NSArray arrayWithObjects:[UIColor blackColor], [UIColor colorWithWhite:0.09 alpha:1.0], nil];
    
}

- (NSArray*)backgroundGradientColorLocations {
    
    return [NSArray arrayWithObjects:[NSNumber numberWithDouble:0.0], [NSNumber numberWithDouble:1.0], nil];
    
}

@end

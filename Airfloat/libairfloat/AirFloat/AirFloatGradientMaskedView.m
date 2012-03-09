//
//  AirFloatGradiantMaskedView.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "AirFloatGradientMaskedView.h"

@implementation AirFloatGradientMaskedView

- (void)awakeFromNib {
    
    CAGradientLayer* gradientLayer = [[CAGradientLayer alloc] init];
    
    gradientLayer.colors = [NSArray arrayWithObjects:(id)[UIColor colorWithWhite:1.0 alpha:1.0].CGColor, (id)[UIColor colorWithWhite:1.0 alpha:0.0].CGColor, nil];
    gradientLayer.locations = [NSArray arrayWithObjects:[NSNumber numberWithDouble:0.0], [NSNumber numberWithDouble:1.0], nil];
    
    gradientLayer.frame = self.bounds;
    
    self.layer.mask = gradientLayer;
    
    [gradientLayer release];
    
}

@end

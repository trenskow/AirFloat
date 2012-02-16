//
//  UIGradientView.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "AirFloatGradientView.h"

@interface AirFloatGradientView (Private)

- (void)_updateGradient;

@end

@implementation AirFloatGradientView

#pragma mark Class Methods

+ (Class)layerClass {
    
    return [CAGradientLayer class];
    
}

#pragma mark Allocation / Deallocation / Load / Unload

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame]))
        [self awakeFromNib];
    
    return self;
    
}

- (void)dealloc {
    
    [_backgroundGradientColors release];
    [_backgroundGradientColorLocations release];
    
    [super dealloc];
    
}

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    [self _updateGradient];
    
}

#pragma mark Property Methods

- (NSArray*)backgroundGradientColors {
    
    if (!_backgroundGradientColors)
        return [NSArray arrayWithObjects:[UIColor blackColor], [UIColor colorWithWhite:0.09 alpha:1.0], nil];
    
    return _backgroundGradientColors;
    
}

- (void)setBackgroundGradientColors:(NSArray *)backgroundGradientColors {
    
    [_backgroundGradientColors release];
    _backgroundGradientColors = [backgroundGradientColors retain];
    
    [self _updateGradient];
    
}

- (NSArray*)backgroundGradientColorLocations {
    
    if (!_backgroundGradientColorLocations)
        return [NSArray arrayWithObjects:[NSNumber numberWithDouble:0.0], [NSNumber numberWithDouble:1.0], nil];
    
    return _backgroundGradientColorLocations;
    
}

- (void)setBackgroundGradientColorLocations:(NSArray *)backgroundGradientColorLocations {
    
    [_backgroundGradientColorLocations release];
    _backgroundGradientColorLocations = [backgroundGradientColorLocations retain];
    
    [self _updateGradient];
    
}

#pragma mark Private Methods

- (void)_updateGradient {
    
    NSArray* colors = [self backgroundGradientColors];
    CAGradientLayer* layer = (CAGradientLayer*)self.layer;
    
    layer.locations = [self backgroundGradientColorLocations];
    
    NSMutableArray* cgColors = [NSMutableArray arrayWithCapacity:[colors count]];
    for (UIColor* color in colors)
        [cgColors addObject:(id)color.CGColor];
    
    layer.colors = cgColors;
    
    [self setNeedsDisplay];
    
}

@end

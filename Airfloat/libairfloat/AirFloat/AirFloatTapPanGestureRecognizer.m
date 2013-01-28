//
//  AirFloatTapPanGestureRecognizer.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/27/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatTapPanGestureRecognizer.h"

@interface AirFloatTapPanGestureRecognizer (Private)

@property (nonatomic,readwrite) UIGestureRecognizerState state;

@end

@implementation AirFloatTapPanGestureRecognizer

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    
    self.state = UIGestureRecognizerStateBegan;
    
}

- (void)touchedMoved:(NSSet *)touches withEvent:(UIEvent*)event {
    
    self.state = UIGestureRecognizerStateChanged;
    
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    
    self.state = UIGestureRecognizerStateEnded;
    
}

@end

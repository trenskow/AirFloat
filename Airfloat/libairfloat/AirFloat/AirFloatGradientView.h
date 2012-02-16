//
//  UIGradientView.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AirFloatGradientView : UIView {
    
    NSArray* _backgroundGradientColors;
    NSArray* _backgroundGradientColorLocations;
    
}

@property (nonatomic,retain) NSArray* backgroundGradientColors;
@property (nonatomic,retain) NSArray* backgroundGradientColorLocations;

@end

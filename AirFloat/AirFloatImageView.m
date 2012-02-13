//
//  AirFloatImageView.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/10/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import "AirFloatImageView.h"

@implementation AirFloatImageView

@synthesize fullsizeImage=_fullsizeImage, flipped=_flipped;

- (void)dealloc {
    
    [_fullsizeImage release];
    
    [super dealloc];
    
}

- (void)setImage:(UIImage*)image {
    
    if (image) {
        
        UIGraphicsBeginImageContextWithOptions(self.bounds.size, YES, self.window.screen.scale);
        CGContextRef context = UIGraphicsGetCurrentContext();
        UIGraphicsPushContext(context);
        
        if (self.isFlipped) {
            CGContextTranslateCTM(context, 0.0, self.bounds.size.height);
            CGContextScaleCTM(context, 1.0, -1.0);
        }
        
        [image drawInRect:self.bounds];
        
        UIGraphicsPopContext();
        [super setImage:UIGraphicsGetImageFromCurrentImageContext()];
        UIGraphicsEndImageContext();
        
    } else {
        [super setImage:[UIImage imageNamed:@"NoArtwork.png"]];
    }

    [_fullsizeImage release];
    _fullsizeImage = [image retain];
    
}

@end

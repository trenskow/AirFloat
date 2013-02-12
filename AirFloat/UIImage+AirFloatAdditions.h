//
//  UIImage+AirFloatAdditions.h
//  
//
//  Created by Kristian Trenskow on 3/11/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIImage (AirFloatAdditions)

+ (UIImage*)imageWithSolidColor:(UIColor*)color withSize:(CGSize)size andScale:(CGFloat)scale;
+ (UIImage*)imageWithSolidColor:(UIColor*)color withSize:(CGSize)size;

- (UIImage*)stackBlur:(NSUInteger)inradius;
- (UIImage*)verticallyFlippedImage;
- (UIImage*)imageByApplyingMask:(UIImage*)mask;
- (UIImage*)imageWithScale:(CGFloat)scale;
- (UIImage*)imageResizedToSize:(CGSize)size;
- (UIImage*)imageScaledToSize:(CGSize)size;

@end

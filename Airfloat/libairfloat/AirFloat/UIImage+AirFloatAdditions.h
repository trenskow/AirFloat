//
//  UIImage+AirFloatAdditions.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/20/13.
//
//

#import <UIKit/UIKit.h>

@interface UIImage (AirFloatAdditions)

- (UIImage*)imageAspectedFilledWithSize:(CGSize)size;
- (UIImage*)imageGaussianBlurredWithRadius:(CGFloat)radius;

@end

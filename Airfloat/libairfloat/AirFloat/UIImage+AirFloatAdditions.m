//
//  UIImage+AirFloatAdditions.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/20/13.
//
//

#import <AVFoundation/AVFoundation.h>

#import "UIImage+AirFloatAdditions.h"

@implementation UIImage (AirFloatAdditions)

- (UIImage *)imageAspectedFilledWithSize:(CGSize)size {
    
    CGSize aspect = CGSizeMake(size.width / self.size.width,
                               size.height / self.size.height);
    
    CGSize newSize = CGSizeMake(self.size.width * MAX(aspect.width, aspect.height),
                                self.size.height * MAX(aspect.width, aspect.height));
    
    if (newSize.width != self.size.width || newSize.height != self.size.height) {
        
        UIGraphicsBeginImageContextWithOptions(size, YES, self.scale);
        
        CGContextRef context = UIGraphicsGetCurrentContext();
        
        CGContextTranslateCTM(context, 0.0, size.height);
        CGContextScaleCTM(context, 1.0, -1.0);
        
        CGContextDrawImage(context, CGRectMake((size.width - newSize.width) / 2, (size.height - newSize.height) / 2, newSize.width, newSize.height), self.CGImage);
        
        UIImage* ret = UIGraphicsGetImageFromCurrentImageContext();
        
        UIGraphicsEndImageContext();
        
        return ret;
        
    }
    
    return self;
    
}

- (UIImage *)imageGaussianBlurredWithRadius:(CGFloat)radius {
    
    if ([[UIDevice currentDevice].systemVersion floatValue] >= 6 && [UIApplication sharedApplication].applicationState == UIApplicationStateActive) {
        
        @try {
            
            EAGLContext* glContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
            CIContext* ciContext = [CIContext contextWithEAGLContext:glContext];
            [glContext release];
            
            CGAffineTransform transform = CGAffineTransformIdentity;
            
            CIImage *inputImage = [CIImage imageWithCGImage:self.CGImage];
            
            CIFilter* clampFilter = [CIFilter filterWithName:@"CIAffineClamp"];
            [clampFilter setDefaults];
            [clampFilter setValue:[NSValue valueWithBytes:&transform objCType:@encode(CGAffineTransform)]
                           forKey:@"inputTransform"];
            [clampFilter setValue:inputImage
                           forKey:kCIInputImageKey];
            
            CIImage* clampedImage = [clampFilter outputImage];
            
            CIFilter *filter = [CIFilter filterWithName:@"CIGaussianBlur"
                                          keysAndValues:kCIInputImageKey, clampedImage,
                                @"inputRadius", [NSNumber numberWithDouble:radius],
                                nil];
            
            CIImage *outputImage = [filter outputImage];
            
            CGImageRef outImage = [ciContext createCGImage:outputImage
                                                  fromRect:[inputImage extent]];
            
            return [UIImage imageWithCGImage:outImage];
            
        }
        @catch (NSException *exception) {
            return nil;
        }
        
    }
    
    return nil;
    
}

@end

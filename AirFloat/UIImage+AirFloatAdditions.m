//
//  UIImage+AirFloatAdditions.m
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#import <AVFoundation/AVFoundation.h>

#import "UIImage+AirFloatAdditions.h"

@implementation UIImage (AirFloatAdditions)

- (UIImage *)imageWithScale:(CGFloat)scale {
    
    return [UIImage imageWithCGImage:self.CGImage scale:scale orientation:self.imageOrientation];
    
}

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

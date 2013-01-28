//
//  AirFloatImageView.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/10/12.
//  Copyright (c) BLUR_RADIUS12 The Famous Software Company. All rights reserved.
//

#import "AirFloatAdditions.h"
#import "AirFloatImageView.h"

#define BLUR_RADIUS 10.0

@interface AirFloatImageView (Private)

@property (nonatomic,readonly) BOOL _canBlur;

- (void)_internalSetImage:(UIImage*)image;

@end

@implementation AirFloatImageView

#pragma mark - Allocation / Deallocation

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame]))
        [self awakeFromNib];
    
    return self;
    
}

- (void)dealloc {
    
    dispatch_release(_imageManipulationQueue);
    
    [_blurredImageView release];
    [_flippedBlurredImageView release];
    
    [_fullsizeImage release];
    
    [super dealloc];
    
}

#pragma mark - Public Methods

- (void)awakeFromNib {
    
    _imageManipulationQueue = dispatch_queue_create("com.AirFloat.imageManipulationQueue", DISPATCH_QUEUE_CONCURRENT);
    
    _blurredImageView = [[UIImageView alloc] initWithFrame:self.bounds];
    _blurredImageView.hidden = YES;
    _blurredImageView.userInteractionEnabled = NO;
    _blurredImageView.contentMode = self.contentMode;
    _blurredImageView.autoresizingMask = self.autoresizingMask;
    [self addSubview:_blurredImageView];
    
    if (self.flippedMirroredImageView) {
        _flippedBlurredImageView = [[UIImageView alloc] initWithFrame:self.bounds];
        _flippedBlurredImageView.hidden = YES;
        _flippedBlurredImageView.userInteractionEnabled = NO;
        [self.flippedMirroredImageView addSubview:_flippedBlurredImageView];
    }
    
}

#pragma mark - Private Methods

- (void)_internalSetImage:(UIImage *)image {
    
    if (dispatch_get_current_queue() != _imageManipulationQueue) {
        dispatch_async(_imageManipulationQueue, ^{
            [self _internalSetImage:image];
        });
        return;
    }
    
    UIImage* scaledImage = [image imageWithScale:self.window.screen.scale];
    
    CGSize maxSize = self.bounds.size;
    
    maxSize.width -= 20;
    maxSize.height -= 20;
    
    CGSize newSize = CGSizeMake(maxSize.width,
                                scaledImage.size.height * (maxSize.width / scaledImage.size.width));
    
    if (newSize.height > maxSize.height)
        newSize = CGSizeMake(scaledImage.size.width * (maxSize.height / scaledImage.size.height),
                             maxSize.height);
    
    UIGraphicsBeginImageContextWithOptions(newSize, NO, self.window.screen.scale);
    CGContextRef context = UIGraphicsGetCurrentContext();
    UIGraphicsPushContext(context);
    
    [scaledImage drawInRect:CGRectMake(0, 0, newSize.width, newSize.height)];
    
    UIGraphicsPopContext();
    
    scaledImage = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();

    UIImage* flippedScaledImage = nil;
    UIImage* blurredScaledImage = nil;
    UIImage* flippedBlurredScaledImage = nil;
    
    UIGraphicsEndImageContext();
    
    if (self.flippedMirroredImageView)
        flippedScaledImage = [scaledImage verticallyFlippedImage];
    
    if (self.generateBlurredImage) {
        blurredScaledImage = [scaledImage stackBlur:BLUR_RADIUS * self.window.screen.scale];
        if (self.flippedMirroredImageView)
            flippedBlurredScaledImage = [blurredScaledImage verticallyFlippedImage];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [super setImage:scaledImage];
        _blurredImageView.image = blurredScaledImage;
        self.flippedMirroredImageView.image = flippedScaledImage;
        _flippedBlurredImageView.image = flippedBlurredScaledImage;
    });
    
}

#pragma mark - Private Properties

- (BOOL)_canBlur {
    
    if ([UICurrentDevice.platformName isEqualToString:@"iPhone"])
        return (UICurrentDevice.platformVersion >= 3.0); // iPhone 4
    else if ([UICurrentDevice.platformName isEqualToString:@"iPad"])
        return (UICurrentDevice.platformVersion >= 2.0); // iPad 2
    else if ([UICurrentDevice.platformName isEqualToString:@"iPod"])
        return (UICurrentDevice.platformVersion >= 4.0); // iPod 4th generation
    
    return YES;    
    
}

#pragma mark - Public Properties

@synthesize fullsizeImage=_fullsizeImage, flippedMirroredImageView=_flippedMirroredImageView;

- (void)setImage:(UIImage*)image {
    
    if (image)
        [self _internalSetImage:image];
    else
        [self _internalSetImage:[UIImage imageNamed:@"NoArtwork.png"]];
        
    [_fullsizeImage release];
    _fullsizeImage = [image retain];
    
}

@synthesize blur=_blur;

- (void)setBlur:(BOOL)blur {
    
    [self setBlur:blur animated:NO slow:NO];
    
}

- (void)setBlur:(BOOL)blur animated:(BOOL)animated {
    
    [self setBlur:blur animated:animated slow:NO];
    
}

- (void)setBlur:(BOOL)blur animated:(BOOL)animated slow:(BOOL)slowAnimated {
    
    _blur = blur;
    if ((blur && self.generateBlurredImage) || !blur) {
        
        if (blur) {
            _blurredImageView.alpha = _flippedBlurredImageView.alpha = 0.0;
            _blurredImageView.hidden = _flippedBlurredImageView.hidden = NO;
        }
        
        [UIView animateWithDuration:(animated ? (slowAnimated ? 3.0 : 0.3) : 0.0)
                              delay:0.0 
                            options:UIViewAnimationCurveEaseOut
                         animations:^{
                             _blurredImageView.alpha = _flippedBlurredImageView.alpha = (blur ? 1.0 : 0.0);
                         } completion:^(BOOL finished) {
                             if (!blur) {
                                 _blurredImageView.hidden = _flippedBlurredImageView.hidden = YES;
                                 _blurredImageView.alpha = _flippedBlurredImageView.alpha = 1.0;
                             }
                         }];
    } else if (blur && !self.generateBlurredImage)
        self.generateBlurredImage = YES;
    
}

@synthesize generateBlurredImage=_generateBlurredImage;

- (void)setGenerateBlurredImage:(BOOL)generateBlurredImage {
    
    if (self._canBlur && generateBlurredImage != _generateBlurredImage) {

        _generateBlurredImage = generateBlurredImage;
        if (_generateBlurredImage && !_blurredImageView.image)
            [self setImage:_fullsizeImage];
        else if (!_generateBlurredImage && _blurredImageView.image) {
            _blurredImageView.image = nil;
            _flippedBlurredImageView.image = nil;
            self.blur = _blur;
        }
    
    }
    
}

@end

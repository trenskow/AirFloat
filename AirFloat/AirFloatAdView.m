//
//  AirFloatAdView.m
//  AdView
//
//  Created by Kristian Trenskow on 2/28/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "AirFloatAdditions.h"
#import "AirFloatFlipInView.h"
#import "AirFloatAdView.h"

@interface AirFloatAdView (Private)

- (void)_displayNextImage;

@end

@implementation AirFloatAdView

#pragma mark - Allocation / Deallocation

- (id)initWithFrame:(CGRect)frame {
    
    if ((self = [super initWithFrame:frame]))
        [self awakeFromNib];
    
    return self;
    
}

- (void)dealloc {
    
    [_images release];
    
    [super dealloc];
    
}

#pragma mark - Public Method

- (void)awakeFromNib {
    
    _currentImage = 0;
    
}

#pragma mark - Public Properties

@synthesize isAnimating=_isAnimating;
@synthesize images=_images;

- (void)setImages:(NSArray *)images {
    
    _currentImage = 0;
    _images = [images retain];
    
}

#pragma mark - Public Methods

- (void)startAnimation {
    
    if (!_isAnimating) {
        _isAnimating = YES;
        [self _displayNextImage];
    }
    
}

- (void)stopAnimation {
    
    if (_isAnimating) {
        
        [NSObject cancelPreviousPerformRequestsWithTarget:self];

        [UIView animateWithDuration:0.3
                              delay:0.0
                            options:UIViewAnimationCurveEaseOut
                         animations:^{
                             self.alpha = 0.0;
                         } completion:^(BOOL finished) {
                             while ([self.subviews count] > 0)
                                 [[self.subviews objectAtIndex:0] removeFromSuperview];
                             _currentImage = 0;
                             _isAnimating = NO;
                             self.alpha = 1.0;
                             self.hidden = YES;
                         }];
        
    }
    
}

#pragma mark - Private Methods

- (void)_displayNextImage {
    
    self.hidden = NO;
    
    NSDictionary* image = [self.images objectAtIndex:_currentImage++];
    
    if (_currentImage == [self.images count])
        _currentImage = 0;
    
    UIView* nextView = [[UIView alloc] initWithFrame:self.bounds];
    
    UIImageView* imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:[image objectForKey:@"AirFloatAdImage"]]];
    
    CGRect frame = imageView.frame;
    imageView.layer.anchorPoint = CGPointMake([[[image objectForKey:@"AirFloatAdImageCenter"] objectForKey:@"x"] doubleValue] / imageView.frame.size.width, 
                                              [[[image objectForKey:@"AirFloatAdImageCenter"] objectForKey:@"y"] doubleValue] / imageView.frame.size.height);
    imageView.frame = frame;
    imageView.alpha = 0.0;
    
    [nextView addSubview:imageView];
    
    CGFloat startScale = 1.1 - ((CGFloat)(arc4random() % 10) / 50.0);
    imageView.layer.transform = CATransform3DMakeScale(startScale, startScale, 1.0);
    
    CGPoint startCenter = CGPointMake([[[image objectForKey:@"AirFloatAdScreenCenter"] objectForKey:@"x"] doubleValue] * self.frame.size.width, 
                                      [[[image objectForKey:@"AirFloatAdScreenCenter"] objectForKey:@"y"] doubleValue] * self.frame.size.height);
    
    CGPoint endCenter = startCenter;
    
    startCenter.x += (CGFloat)(arc4random() % 50) - 25.0;
    startCenter.y += (CGFloat)(arc4random() % 50) - 25.0;
    imageView.center = startCenter;
    
    if (imageView.frame.origin.x > 0)
        startCenter.x -= imageView.frame.origin.x;
    if (imageView.frame.origin.y > 0)
        startCenter.y -= imageView.frame.origin.y;
    
    imageView.center = startCenter;
    
    UIView* textView = [[UIView alloc] initWithFrame:self.bounds];
    [nextView addSubview:textView];
    
    NSArray* adText = [image objectForKey:@"AirFloatAdTexts"];
    
    for (NSInteger i = 0 ; i < [adText count] ; i++) {
        
        UILabel* label = [[UILabel alloc] initWithFrame:CGRectZero];
        label.text = [[adText objectAtIndex:i] objectForKey:@"AirFloatAdText"];
        label.font = [UIFont fontWithName:@"HelveticaNeue-UltraLight" size:36];
        if ([UICurrentDevice.systemVersion doubleValue] < 5)
            label.font = [UIFont fontWithName:@"HelveticaNeue" size:36];
        if ([[[adText objectAtIndex:i] objectForKey:@"AirFloatAdTextIsBold"] boolValue])
            label.font = [UIFont fontWithName:@"HelveticaNeue-Bold" size:36];
        
        label.textColor = [UIColor whiteColor];
        label.backgroundColor = [UIColor blackColor];
        
        [label sizeToFit];
        
        CGRect flipFrame = label.frame;
        flipFrame.size.width = flipFrame.size.width * 1.5;
        flipFrame.size.height = 46;
        
        if ([[[image objectForKey:@"AirFloatAdTextPosition"] objectForKey:@"AirFloatVertical"] isEqualToString:@"bottom"])
            flipFrame.origin.y = self.frame.size.height - (([adText count] - i) * (flipFrame.size.height + 8)) - 2;
        else
            flipFrame.origin.y = (i * (flipFrame.size.height + 8)) + 30;
        
        if ([[[image objectForKey:@"AirFloatAdTextPosition"] objectForKey:@"AirFloatHorizontal"] isEqualToString:@"left"])
            flipFrame.origin.x = 10;
        else
            flipFrame.origin.x = self.frame.size.width - flipFrame.size.width - 10;
        
        AirFloatFlipInView* flipView = [[AirFloatFlipInView alloc] initWithFrame:flipFrame];
        flipView.backgroundColor = [UIColor blackColor];
        
        if ([[image objectForKey:@"AirFloatAdTextColor"] isEqualToString:@"black"]) {
            label.textColor = [UIColor blackColor];
            flipView.backgroundColor = label.backgroundColor = [UIColor whiteColor];
        }
        
        CGRect labelFrame = label.frame;

        if ([[[image objectForKey:@"AirFloatAdTextPosition"] objectForKey:@"AirFloatHorizontal"] isEqualToString:@"left"])
            labelFrame.origin.x = 10;
        else
            labelFrame.origin.x = flipFrame.size.width - labelFrame.size.width - 10;
        
        labelFrame.size.height = 46;
        label.frame = labelFrame;
        
        [flipView addSubview:label];
        
        [textView addSubview:flipView];
        
        [flipView performSelector:@selector(flip:) withObject:nil afterDelay:[[[adText objectAtIndex:i] objectForKey:@"AirFloatAdDelay"] doubleValue] + 2];
        
        [label release];
        [flipView release];
        
    }
    
    UIView* oldView = nil;
    if ([self.subviews count] > 0)
        oldView = [self.subviews objectAtIndex:0];
    
    NSTimeInterval duration = [[image objectForKey:@"AirFloatAdDuration"] doubleValue];
    
    [UIView animateWithDuration:1.0
                          delay:0.0
                        options:UIViewAnimationCurveEaseInOut
                     animations:^{
                         imageView.alpha = 1.0;
                     } completion:^(BOOL finished) {
                         [oldView removeFromSuperview];
                     }];
    
    [UIView animateWithDuration:duration
                          delay:0.0
                        options:UIViewAnimationCurveEaseOut
                     animations:^{
                         imageView.layer.transform = CATransform3DMakeScale(1.0, 1.0, 1.0);
                         imageView.center = endCenter;
                     } completion:NULL];
    
    [self performSelector:@selector(_displayNextImage) withObject:nil afterDelay:duration - 1.0];
    
    [self addSubview:nextView];
    
    [imageView release];
    [textView release];
    [nextView release];
    
}

@end

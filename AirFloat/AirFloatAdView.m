//
//  AirFloatAdView.m
//  AdView
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

#import <QuartzCore/QuartzCore.h>

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
    
    for (NSTimer* timer in _timers)
        [timer invalidate];
    
}

#pragma mark - Public Method

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    _currentImage = 0;
    _timers = [[NSMutableArray alloc] init];
    
}

#pragma mark - Public Properties

@synthesize isAnimating=_isAnimating;
@synthesize images=_images;

- (void)setImages:(NSArray *)images {
    
    _currentImage = 0;
    _images = images;
    
}

#pragma mark - Public Methods

- (void)startAnimation {
    
    if (!_isAnimating) {
        _isAnimating = YES;
        // Wait a while. Framing must be in place before starting.
        [self performSelector:@selector(_displayNextImage) withObject:nil afterDelay:0.1];
    }
    
}

- (void)stopAnimation {
    
    if (_isAnimating) {
        
        for (NSTimer* timer in _timers)
            [timer invalidate];
        
        _timers = [[NSMutableArray alloc] init];
        
        [UIView animateWithDuration:1.0
                              delay:0.0
                            options:UIViewAnimationOptionCurveEaseOut
                         animations:^{
                             self.alpha = 0.0;
                         } completion:^(BOOL finished) {
                             while ([self.subviews count] > 0)
                                 [[self.subviews objectAtIndex:0] removeFromSuperview];
                             self->_currentImage = 0;
                             self->_isAnimating = NO;
                             self.alpha = 1.0;
                             self.hidden = YES;
                         }];
        
    }
    
}

#pragma mark - Private Methods

- (void)_invalidateTimer:(NSTimer *)timer {
    
    [timer invalidate];
    [_timers removeObject:timer];
    
}

- (void)_doFlip:(NSTimer*)timer {
    
    AirFloatFlipInView* flipView = [timer userInfo];
    
    [flipView flip:nil];
    
    [self _invalidateTimer:timer];
    
}

- (void)_doDisplayNextImage:(NSTimer *)timer {
    
    [self _displayNextImage];
    
    [self _invalidateTimer:timer];
    
}

- (void)_displayNextImage {
    
    self.hidden = NO;
    
    NSDictionary* image = [self.images objectAtIndex:_currentImage++];
    
    if (_currentImage == [self.images count])
        _currentImage = 0;
    
    UIView* nextView = [[UIView alloc] initWithFrame:self.bounds];
    
    UIImageView* imageView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:[image objectForKey:@"AirFloatAdImage"]]];
    
    CGRect frame = imageView.frame;
    
    CGPoint anchorPoint = CGPointMake([[[image objectForKey:@"AirFloatAdImageCenter"] objectForKey:@"x"] doubleValue] / imageView.frame.size.width,
                                              [[[image objectForKey:@"AirFloatAdImageCenter"] objectForKey:@"y"] doubleValue] / imageView.frame.size.height);
    imageView.layer.anchorPoint = anchorPoint;
    
    imageView.frame = frame;
    imageView.alpha = 0.0;
    
    [nextView addSubview:imageView];
    
    CGFloat startScale = 1.1 - ((CGFloat)(arc4random() % 10) / 50.0);
    CGFloat endScale = 1.0;
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        endScale = MAX(self.bounds.size.width / frame.size.width, self.bounds.size.height / frame.size.height);
        startScale = endScale + ((CGFloat)(arc4random() % 10) / 50.0);
    }
    
    imageView.layer.transform = CATransform3DMakeScale(startScale, startScale, 1.0);
    
    CGPoint startCenter = CGPointMake([[[image objectForKey:@"AirFloatAdScreenCenter"] objectForKey:@"x"] doubleValue] * self.bounds.size.width,
                                      [[[image objectForKey:@"AirFloatAdScreenCenter"] objectForKey:@"y"] doubleValue] * self.bounds.size.height);
    
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
    
    nextView.autoresizingMask = textView.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    
    NSArray* adText = [image objectForKey:@"AirFloatAdTexts"];
    
    for (NSInteger i = 0 ; i < [adText count] ; i++) {
        
        CGFloat fontSize = ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone ? 36 : 52);
        
        UILabel* label = [[UILabel alloc] initWithFrame:CGRectZero];
        label.text = [[adText objectAtIndex:i] objectForKey:@"AirFloatAdText"];
        label.font = [UIFont fontWithName:@"HelveticaNeue-UltraLight" size:fontSize];
        if ([[UIDevice currentDevice].systemVersion doubleValue] < 5)
            label.font = [UIFont fontWithName:@"HelveticaNeue" size:fontSize];
        if ([[[adText objectAtIndex:i] objectForKey:@"AirFloatAdTextIsBold"] boolValue])
            label.font = [UIFont fontWithName:@"HelveticaNeue-Bold" size:fontSize];
        
        label.textColor = [UIColor whiteColor];
        label.backgroundColor = [UIColor blackColor];
        
        [label sizeToFit];
        
        CGRect flipFrame = label.frame;
        flipFrame.size.width = flipFrame.size.width * 1.5;
        flipFrame.size.height = fontSize + 10;
        
        UIViewAutoresizing autoresizingMask = UIViewAutoresizingNone;
        
        if ([[[image objectForKey:@"AirFloatAdTextPosition"] objectForKey:@"AirFloatVertical"] isEqualToString:@"bottom"]) {
            flipFrame.origin.y = self.frame.size.height - (([adText count] - i) * (flipFrame.size.height + 8)) - 2;
            autoresizingMask |= UIViewAutoresizingFlexibleTopMargin;
        } else {
            flipFrame.origin.y = (i * (flipFrame.size.height + 8)) + 10;
            autoresizingMask |= UIViewAutoresizingFlexibleBottomMargin;
        }
        
        if ([[[image objectForKey:@"AirFloatAdTextPosition"] objectForKey:@"AirFloatHorizontal"] isEqualToString:@"left"]) {
            flipFrame.origin.x = 10;
            autoresizingMask |= UIViewAutoresizingFlexibleRightMargin;
        } else {
            flipFrame.origin.x = self.frame.size.width - flipFrame.size.width - 10;
            autoresizingMask |= UIViewAutoresizingFlexibleLeftMargin;
        }
        
        AirFloatFlipInView* flipView = [[AirFloatFlipInView alloc] initWithFrame:flipFrame];
        flipView.backgroundColor = [UIColor blackColor];
        flipView.autoresizingMask = autoresizingMask;
        
        if ([[image objectForKey:@"AirFloatAdTextColor"] isEqualToString:@"black"]) {
            label.textColor = [UIColor blackColor];
            flipView.backgroundColor = label.backgroundColor = [UIColor whiteColor];
        }
        
        CGRect labelFrame = label.frame;

        if ([[[image objectForKey:@"AirFloatAdTextPosition"] objectForKey:@"AirFloatHorizontal"] isEqualToString:@"left"])
            labelFrame.origin.x = 10;
        else
            labelFrame.origin.x = flipFrame.size.width - labelFrame.size.width - 10;
        
        labelFrame.size.height = fontSize + 10;
        label.frame = labelFrame;
        
        [flipView addSubview:label];
        
        [textView addSubview:flipView];
        
        NSTimer* timer = [[NSTimer alloc] initWithFireDate:[NSDate dateWithTimeIntervalSinceNow:[[[adText objectAtIndex:i] objectForKey:@"AirFloatAdDelay"] doubleValue] + 2]
                                                  interval:0
                                                    target:self
                                                  selector:@selector(_doFlip:)
                                                  userInfo:flipView
                                                   repeats:NO];
        
        [_timers addObject:timer];
        
        [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
        
    }
    
    UIView* oldView = nil;
    if ([self.subviews count] > 0)
        oldView = [self.subviews objectAtIndex:0];
    
    NSTimeInterval duration = [[image objectForKey:@"AirFloatAdDuration"] doubleValue];
    
    [UIView animateWithDuration:1.0
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseInOut
                     animations:^{
                         imageView.alpha = 1.0;
                         oldView.alpha = 0.0;
                     } completion:^(BOOL finished) {
                         [oldView removeFromSuperview];
                     }];
    
    [UIView animateWithDuration:duration
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         imageView.layer.transform = CATransform3DMakeScale(endScale, endScale, 1.0);
                         imageView.center = endCenter;
                     } completion:NULL];
    
    NSTimer* timer = [[NSTimer alloc] initWithFireDate:[NSDate dateWithTimeIntervalSinceNow:duration - 1.0]
                                              interval:0.0
                                                target:self
                                              selector:@selector(_doDisplayNextImage:)
                                              userInfo:nil
                                               repeats:NO];
    
    [_timers addObject:timer];
    
    [[NSRunLoop mainRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
    
    [self addSubview:nextView];
    
}

- (void)setFrame:(CGRect)frame {
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        
        CGRect oldFrame = self.frame;
        
        CGPoint frameDiff = CGPointMake(round((frame.size.width - oldFrame.size.width) / 2),
                                        round((frame.size.height - oldFrame.size.height) / 2));
        
        for (UIView* view in self.subviews) {
            
            UIView* imageView = [view.subviews objectAtIndex:0];
            
            CGPoint center = imageView.center;
            center.x += frameDiff.x;
            center.y += frameDiff.y;
            imageView.center = center;
            
            CGFloat scale = MAX(frame.size.width / imageView.bounds.size.width, frame.size.height / imageView.bounds.size.height);
            
            imageView.layer.transform = CATransform3DMakeScale(scale, scale, 1.0);
            
        }
        
    }
    
    [super setFrame:frame];
    
}

@end

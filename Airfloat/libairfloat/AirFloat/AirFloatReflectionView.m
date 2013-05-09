//
//  AirFloatReflectionView.m
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

#import <QuartzCore/QuartzCore.h>

#import "AirFloatReflectionView.h"

@implementation AirFloatReflectionView

+ (Class)layerClass {
    
    return [CAReplicatorLayer class];
    
}

- (void)awakeFromNib {
    
    [super awakeFromNib];
    
    [self setFrame:self.frame];
    
}

- (void)setFrame:(CGRect)frame {
    
    [super setFrame:frame];
    
    CAReplicatorLayer* layer = (CAReplicatorLayer *)self.layer;
    
    [UIView setAnimationsEnabled:NO];
    
    CATransform3D transform = CATransform3DTranslate(CATransform3DMakeScale(1.0, -1.0, 1.0), 0.0, -frame.size.height, 0.0);
    
    layer.instanceCount = 2;
    layer.instanceTransform = transform;
    
    [UIView setAnimationsEnabled:YES];
    
}

@end

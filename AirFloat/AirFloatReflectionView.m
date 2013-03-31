//
//  AirFloatReflectionView.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/20/13.
//
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
    
    CATransform3D transform = CATransform3DTranslate(CATransform3DMakeScale(1.0, -1.0, 1.0), 0.0, -frame.size.height, 0.0);
    
    layer.instanceCount = 2;
    layer.instanceTransform = transform;
    
}

@end

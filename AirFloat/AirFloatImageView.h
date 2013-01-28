//
//  AirFloatImageView.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/10/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AirFloatImageView : UIImageView {
    
    UIImage* _fullsizeImage;
    
    UIImageView* _blurredImageView;
    UIImageView* _flippedBlurredImageView;
    
    BOOL _generateBlurredImage;
    BOOL _blur;
    
    dispatch_queue_t _imageManipulationQueue;
    
    UIImageView* _flippedMirroredImageView;
    
}

@property (readonly) UIImage* fullsizeImage;
@property (nonatomic,assign) BOOL generateBlurredImage;
@property (nonatomic,assign) BOOL blur;

@property (nonatomic,retain) IBOutlet UIImageView* flippedMirroredImageView;

- (void)setBlur:(BOOL)blur animated:(BOOL)animated;
- (void)setBlur:(BOOL)blur animated:(BOOL)animated slow:(BOOL)slowAnimated;

@end

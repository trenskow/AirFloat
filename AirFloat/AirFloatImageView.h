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
    BOOL _flipped;
    
}

@property (readonly) UIImage* fullsizeImage;
@property (nonatomic,assign,getter=isFlipped) BOOL flipped;

@end

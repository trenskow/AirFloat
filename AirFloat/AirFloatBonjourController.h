//
//  AirFloatBonjourController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/9/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AirFloatBonjourController : NSObject {
    
    NSNetService* _service;
    
}

- (id)initWithMacAddress:(NSString*)macAddress andPort:(NSUInteger)port;

@end

//
//  AirFloatDACPBrowser.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/12/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AirFloatDACPClient.h"

@class AirFloatDACPBrowser;

@protocol AirFloatDACPBrowserDelegate

- (void)dacpBrowser:(AirFloatDACPBrowser*)browser didFindHost:(NSString*)host;

@end

@interface AirFloatDACPBrowser : NSObject <NSNetServiceBrowserDelegate, NSNetServiceDelegate> {
    
    NSNetServiceBrowser* _netServiceBrowser;
    NSMutableArray* _foundDomains;
    NSMutableArray* _foundServices;
    
    NSString* _dacpId;
    
    __weak id<AirFloatDACPBrowserDelegate> _delegate;
    
}

@property (assign) id<AirFloatDACPBrowserDelegate> delegate;

- (void)findServerForIdentifier:(NSString*)dacpId;

@end

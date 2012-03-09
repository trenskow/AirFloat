//
//  AirFloatBonjourBrowser.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/12/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <Foundation/Foundation.h>

@class AirFloatBonjourBrowser;

@protocol AirFloatBonjourBrowserDelegate

- (bool)bonjourBrowser:(AirFloatBonjourBrowser*)browser didFindAddresses:(NSArray*)addresses forService:(NSNetService*)serviceType;
- (void)bonjourBrowser:(AirFloatBonjourBrowser*)browser endedSearchForServiceType:(NSString*)serviceType;

@end

@interface AirFloatBonjourBrowser : NSObject <NSNetServiceBrowserDelegate, NSNetServiceDelegate> {
    
    NSNetServiceBrowser* _netServiceBrowser;
    NSMutableArray* _foundDomains;
    NSMutableArray* _foundServices;
    
    NSString* _serviceType;
    
    __weak id<AirFloatBonjourBrowserDelegate> _delegate;
    
}

@property (assign) id<AirFloatBonjourBrowserDelegate> delegate;

- (void)findService:(NSString*)service;

@end

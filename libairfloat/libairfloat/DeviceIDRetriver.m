// -----------------------------------
//  DeviceIDRetriver.m (ARC-managed)
// -----------------------------------

#import "DeviceIDRetriver.h"
#import <UIKit/UIKit.h>

uint64_t iOSDeviceID() {
    
    uint64_t returned = 0;
    
    // get ID string
    NSString *idString = [[[UIDevice currentDevice] identifierForVendor] UUIDString];
    
    // convert to data, then store sufficient data into uint64_t.
    // this is a crude NSString -> uint64_t hash
    NSData *idData = [idString dataUsingEncoding:NSUTF8StringEncoding];
    [idData getBytes:&returned length:sizeof(uint64_t)];
    
    return returned;
}
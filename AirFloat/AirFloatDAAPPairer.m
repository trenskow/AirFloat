//
//  AirFloatDAAPPairer.m
//  AirFloat
//
//  Created by Kristian Trenskow on 2/14/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#import <assert.h>

#import "AirFloatAdditions.h"
#import "AirFloatServerController.h"
#import "AirFloatDAAPClient.h"
#import "AirFloatDAAPPairer.h"

NSString* _itunesPairingcode(NSString* passcode, NSString* pair) 
{
    
    const char* pc = [passcode cStringUsingEncoding:NSASCIIStringEncoding];
    const char* p = [pair cStringUsingEncoding:NSASCIIStringEncoding];
    
    // initialize the param array
    char param[64];
    memset( param, 0, 64 );
    param[56] = '\xc0';
    param[24] = '\x80';
    
    // set the pair value
    strncpy( param, p, 16 );
    
    // copy the pair code
    strncpy( param+16, pc, 4 );
    param[22] = param[19];
    param[20] = param[18];
    param[18] = param[17];
    param[19] = param[17] = 0;
    
    // initialize a, b, c, d
    uint32_t a = 0x67452301;
    uint32_t b = 0xefcdab89;
    uint32_t c = 0x98badcfe;
    uint32_t d = 0x10325476;
    
    a = ((b & c) | (~b & d)) + (*(uint32_t*)(param+0x00)) + a - 0x28955B88;
    a = ((a << 0x07) | (a >> 0x19)) + b;
    d = ((a & b) | (~a & c)) + (*(uint32_t*)(param+0x04)) + d - 0x173848AA;
    d = ((d << 0x0c) | (d >> 0x14)) + a;
    c = ((d & a) | (~d & b)) + (*(uint32_t*)(param+0x08)) + c + 0x242070DB;
    c = ((c << 0x11) | (c >> 0x0f)) + d;
    b = ((c & d) | (~c & a)) + (*(uint32_t*)(param+0x0c)) + b - 0x3E423112;
    b = ((b << 0x16) | (b >> 0x0a)) + c;
    a = ((b & c) | (~b & d)) + (*(uint32_t*)(param+0x10)) + a - 0x0A83F051;
    a = ((a << 0x07) | (a >> 0x19)) + b;
    d = ((a & b) | (~a & c)) + (*(uint32_t*)(param+0x14)) + d + 0x4787C62A;
    d = ((d << 0x0c) | (d >> 0x14)) + a;
    c = ((d & a) | (~d & b)) + (*(uint32_t*)(param+0x18)) + c - 0x57CFB9ED;
    c = ((c << 0x11) | (c >> 0x0f)) + d;
    b = ((c & d) | (~c & a)) + (*(uint32_t*)(param+0x1c)) + b - 0x02B96AFF;
    b = ((b << 0x16) | (b >> 0x0a)) + c;
    a = ((b & c) | (~b & d)) + (*(uint32_t*)(param+0x20)) + a + 0x698098D8;
    a = ((a << 0x07) | (a >> 0x19)) + b;
    d = ((a & b) | (~a & c)) + (*(uint32_t*)(param+0x24)) + d - 0x74BB0851;
    d = ((d << 0x0c) | (d >> 0x14)) + a;
    c = ((d & a) | (~d & b)) + (*(uint32_t*)(param+0x28)) + c - 0x0000A44F;
    c = ((c << 0x11) | (c >> 0x0f)) + d;
    b = ((c & d) | (~c & a)) + (*(uint32_t*)(param+0x2C)) + b - 0x76A32842;
    b = ((b << 0x16) | (b >> 0x0a)) + c;
    a = ((b & c) | (~b & d)) + (*(uint32_t*)(param+0x30)) + a + 0x6B901122;
    a = ((a << 0x07) | (a >> 0x19)) + b;
    d = ((a & b) | (~a & c)) + (*(uint32_t*)(param+0x34)) + d - 0x02678E6D;
    d = ((d << 0x0c) | (d >> 0x14)) + a;
    c = ((d & a) | (~d & b)) + (*(uint32_t*)(param+0x38)) + c - 0x5986BC72;
    c = ((c << 0x11) | (c >> 0x0f)) + d;
    b = ((c & d) | (~c & a)) + (*(uint32_t*)(param+0x3c)) + b + 0x49B40821;
    b = ((b << 0x16) | (b >> 0x0a)) + c;
    
    a = ((b & d) | (~d & c)) + (*(uint32_t*)(param+0x04)) + a - 0x09E1DA9E;
    a = ((a << 0x05) | (a >> 0x1b)) + b;
    d = ((a & c) | (~c & b)) + (*(uint32_t*)(param+0x18)) + d - 0x3FBF4CC0;
    d = ((d << 0x09) | (d >> 0x17)) + a;
    c = ((d & b) | (~b & a)) + (*(uint32_t*)(param+0x2c)) + c + 0x265E5A51;
    c = ((c << 0x0e) | (c >> 0x12)) + d;
    b = ((c & a) | (~a & d)) + (*(uint32_t*)(param+0x00)) + b - 0x16493856;
    b = ((b << 0x14) | (b >> 0x0c)) + c;
    a = ((b & d) | (~d & c)) + (*(uint32_t*)(param+0x14)) + a - 0x29D0EFA3;
    a = ((a << 0x05) | (a >> 0x1b)) + b;
    d = ((a & c) | (~c & b)) + (*(uint32_t*)(param+0x28)) + d + 0x02441453;
    d = ((d << 0x09) | (d >> 0x17)) + a;
    c = ((d & b) | (~b & a)) + (*(uint32_t*)(param+0x3c)) + c - 0x275E197F;
    c = ((c << 0x0e) | (c >> 0x12)) + d;
    b = ((c & a) | (~a & d)) + (*(uint32_t*)(param+0x10)) + b - 0x182C0438;
    b = ((b << 0x14) | (b >> 0x0c)) + c;
    a = ((b & d) | (~d & c)) + (*(uint32_t*)(param+0x24)) + a + 0x21E1CDE6;
    a = ((a << 0x05) | (a >> 0x1b)) + b;
    d = ((a & c) | (~c & b)) + (*(uint32_t*)(param+0x38)) + d - 0x3CC8F82A;
    d = ((d << 0x09) | (d >> 0x17)) + a;
    c = ((d & b) | (~b & a)) + (*(uint32_t*)(param+0x0c)) + c - 0x0B2AF279;
    c = ((c << 0x0e) | (c >> 0x12)) + d;
    b = ((c & a) | (~a & d)) + (*(uint32_t*)(param+0x20)) + b + 0x455A14ED;
    b = ((b << 0x14) | (b >> 0x0c)) + c;
    a = ((b & d) | (~d & c)) + (*(uint32_t*)(param+0x34)) + a - 0x561C16FB;
    a = ((a << 0x05) | (a >> 0x1b)) + b;
    d = ((a & c) | (~c & b)) + (*(uint32_t*)(param+0x08)) + d - 0x03105C08;
    d = ((d << 0x09) | (d >> 0x17)) + a;
    c = ((d & b) | (~b & a)) + (*(uint32_t*)(param+0x1c)) + c + 0x676F02D9;
    c = ((c << 0x0e) | (c >> 0x12)) + d;
    b = ((c & a) | (~a & d)) + (*(uint32_t*)(param+0x30)) + b - 0x72D5B376;
    b = ((b << 0x14) | (b >> 0x0c)) + c;
    
    a = (b ^ c ^ d) + (*(uint32_t*)(param+0x14)) + a - 0x0005C6BE;
    a = ((a << 0x04) | (a >> 0x1c)) + b;
    d = (a ^ b ^ c) + (*(uint32_t*)(param+0x20)) + d - 0x788E097F;
    d = ((d << 0x0b) | (d >> 0x15)) + a;
    c = (d ^ a ^ b) + (*(uint32_t*)(param+0x2c)) + c + 0x6D9D6122;
    c = ((c << 0x10) | (c >> 0x10)) + d;
    b = (c ^ d ^ a) + (*(uint32_t*)(param+0x38)) + b - 0x021AC7F4;
    b = ((b << 0x17) | (b >> 0x09)) + c;
    a = (b ^ c ^ d) + (*(uint32_t*)(param+0x04)) + a - 0x5B4115BC;
    a = ((a << 0x04) | (a >> 0x1c)) + b;
    d = (a ^ b ^ c) + (*(uint32_t*)(param+0x10)) + d + 0x4BDECFA9;
    d = ((d << 0x0b) | (d >> 0x15)) + a;
    c = (d ^ a ^ b) + (*(uint32_t*)(param+0x1c)) + c - 0x0944B4A0;
    c = ((c << 0x10) | (c >> 0x10)) + d;
    b = (c ^ d ^ a) + (*(uint32_t*)(param+0x28)) + b - 0x41404390;
    b = ((b << 0x17) | (b >> 0x09)) + c;
    a = (b ^ c ^ d) + (*(uint32_t*)(param+0x34)) + a + 0x289B7EC6;
    a = ((a << 0x04) | (a >> 0x1c)) + b;
    d = (a ^ b ^ c) + (*(uint32_t*)(param+0x00)) + d - 0x155ED806;
    d = ((d << 0x0b) | (d >> 0x15)) + a;
    c = (d ^ a ^ b) + (*(uint32_t*)(param+0x0c)) + c - 0x2B10CF7B;
    c = ((c << 0x10) | (c >> 0x10)) + d;
    b = (c ^ d ^ a) + (*(uint32_t*)(param+0x18)) + b + 0x04881D05;
    b = ((b << 0x17) | (b >> 0x09)) + c;
    a = (b ^ c ^ d) + (*(uint32_t*)(param+0x24)) + a - 0x262B2FC7;
    a = ((a << 0x04) | (a >> 0x1c)) + b;
    d = (a ^ b ^ c) + (*(uint32_t*)(param+0x30)) + d - 0x1924661B;
    d = ((d << 0x0b) | (d >> 0x15)) + a;
    c = (d ^ a ^ b) + (*(uint32_t*)(param+0x3c)) + c + 0x1fa27cf8;
    c = ((c << 0x10) | (c >> 0x10)) + d;
    b = (c ^ d ^ a) + (*(uint32_t*)(param+0x08)) + b - 0x3B53A99B;
    b = ((b << 0x17) | (b >> 0x09)) + c;
    
    a = ((~d | b) ^ c) + (*(uint32_t*)(param+0x00)) + a - 0x0BD6DDBC;
    a = ((a << 0x06) | (a >> 0x1a)) + b; 
    d = ((~c | a) ^ b) + (*(uint32_t*)(param+0x1c)) + d + 0x432AFF97;
    d = ((d << 0x0a) | (d >> 0x16)) + a; 
    c = ((~b | d) ^ a) + (*(uint32_t*)(param+0x38)) + c - 0x546BDC59;
    c = ((c << 0x0f) | (c >> 0x11)) + d; 
    b = ((~a | c) ^ d) + (*(uint32_t*)(param+0x14)) + b - 0x036C5FC7;
    b = ((b << 0x15) | (b >> 0x0b)) + c; 
    a = ((~d | b) ^ c) + (*(uint32_t*)(param+0x30)) + a + 0x655B59C3;
    a = ((a << 0x06) | (a >> 0x1a)) + b; 
    d = ((~c | a) ^ b) + (*(uint32_t*)(param+0x0C)) + d - 0x70F3336E;
    d = ((d << 0x0a) | (d >> 0x16)) + a; 
    c = ((~b | d) ^ a) + (*(uint32_t*)(param+0x28)) + c - 0x00100B83;
    c = ((c << 0x0f) | (c >> 0x11)) + d; 
    b = ((~a | c) ^ d) + (*(uint32_t*)(param+0x04)) + b - 0x7A7BA22F;
    b = ((b << 0x15) | (b >> 0x0b)) + c; 
    a = ((~d | b) ^ c) + (*(uint32_t*)(param+0x20)) + a + 0x6FA87E4F;
    a = ((a << 0x06) | (a >> 0x1a)) + b; 
    d = ((~c | a) ^ b) + (*(uint32_t*)(param+0x3c)) + d - 0x01D31920;
    d = ((d << 0x0a) | (d >> 0x16)) + a; 
    c = ((~b | d) ^ a) + (*(uint32_t*)(param+0x18)) + c - 0x5CFEBCEC;
    c = ((c << 0x0f) | (c >> 0x11)) + d; 
    b = ((~a | c) ^ d) + (*(uint32_t*)(param+0x34)) + b + 0x4E0811A1;
    b = ((b << 0x15) | (b >> 0x0b)) + c; 
    a = ((~d | b) ^ c) + (*(uint32_t*)(param+0x10)) + a - 0x08AC817E;
    a = ((a << 0x06) | (a >> 0x1a)) + b; 
    d = ((~c | a) ^ b) + (*(uint32_t*)(param+0x2c)) + d - 0x42C50DCB;
    d = ((d << 0x0a) | (d >> 0x16)) + a; 
    c = ((~b | d) ^ a) + (*(uint32_t*)(param+0x08)) + c + 0x2AD7D2BB;
    c = ((c << 0x0f) | (c >> 0x11)) + d; 
    b = ((~a | c) ^ d) + (*(uint32_t*)(param+0x24)) + b - 0x14792C6F;
    b = ((b << 0x15) | (b >> 0x0b)) + c; 
    
    a += 0x67452301;
    b += 0xefcdab89;
    c += 0x98badcfe;
    d += 0x10325476;
    
    // switch to little endian
    a = ((a&0xff000000)>>24) + ((a&0xff0000)>>8) + ((a&0xff00)<<8) + ((a&0xff)<<24);
    b = ((b&0xff000000)>>24) + ((b&0xff0000)>>8) + ((b&0xff00)<<8) + ((b&0xff)<<24);
    c = ((c&0xff000000)>>24) + ((c&0xff0000)>>8) + ((c&0xff00)<<8) + ((c&0xff)<<24);
    d = ((d&0xff000000)>>24) + ((d&0xff0000)>>8) + ((d&0xff00)<<8) + ((d&0xff)<<24);
    
    return [NSString stringWithFormat:@"%08X%08X%08X%08X", a, b, c, d];
    
}

@implementation AirFloatDAAPPairer

@synthesize delegate=_delegate;

#pragma Allocation / Deallocation

- (id)initWithPasscode:(NSString *)passcode {
    
    if ((self = [self init])) {
        
        _passcode = [passcode retain];
        
        _webServer = [[AirFloatWebServer alloc] init];
        _webServer.delegate = self;
        
        if ([_webServer startServerOnPort:50508 tryPorts:10]) {
        
            _netService = [[NSNetService alloc] initWithDomain:@"local." type:@"_touch-remote._tcp." name:[[NSData dataWithRandomData:20] hexEncodedString] port:_webServer.localEndPoint.port];
            
            NSString* pairCode = [[NSData dataWithRandomData:8] hexEncodedString];
            
            _expectedResponse = [_itunesPairingcode(_passcode, pairCode) retain];
            
            [_netService setTXTRecordData:
             [NSNetService dataFromTXTRecordDictionary:
              [NSDictionary dictionaryWithObjectsAndKeys:
               @"1", @"txtvers",
               pairCode, @"Pair",
               @"Remote", @"RemN",
               @"10000", @"RemV",
               [UIDevice currentDevice].model, @"DvTy",
               [NSString stringWithFormat:@"%@ @ %@", [UIApplication sharedApplication].applicationName, [UIDevice currentDevice].name], @"DvNm",
               nil]]];
            
            [_netService publish];
            
        } else {
            [self release];
            return nil;
        }
        
    }
    
    return self;
    
}

- (void)dealloc {
    
    [_webServer release];
    
    [_loginToken release];
    [_expectedResponse release];
    
    [_netService stop];
    [_netService release];
    [_passcode release];
    
    [super dealloc];
    
}

#pragma mark Web Server Delegate Methods

- (BOOL)server:(AirFloatWebServer *)server shouldAcceptConnection:(AirFloatWebConnection *)connection {
    
    connection.delegate = self;
    
    return YES;
    
}

#pragma mark Web Connection Delegate Methods

- (void)processRequest:(AirFloatWebRequest *)request fromConnection:(AirFloatWebConnection *)connection {
    
    [_loginToken release];
    _loginToken = nil;
    
    NSString* requestedPath = [request.url path];
    
    if ([requestedPath isEqualToString:@"/pair"]) {
        
        NSDictionary* params = [NSDictionary dictionaryWithParametersFromURLQuery:[request.url query]];
        
        NSString* pairingCode;
        NSString* serviceName;
        
        if ((pairingCode = [params objectForKey:@"pairingcode"]) && (serviceName = [params objectForKey:@"servicename"]) && [pairingCode isEqualToString:_expectedResponse]) {
            
            _loginToken = [[AirFloatDAAPClient addService:serviceName] retain];
            
            NSDictionary* responseDictionary = [NSDictionary dictionaryWithObjectsAndKeys:
                                                _loginToken, @"com.AirFloat.PairingDeviceToken",
                                                [UIDevice currentDevice].name, @"com.AirFloat.PairingDeviceName",
                                                [[[UIDevice currentDevice].model componentsSeparatedByString:@" "] objectAtIndex:0], @"com.AirFloat.PairingDeviceType",
                                                nil];
                        
            [request.response setStatusMessage:@"OK" forCode:200];
            [request.response setBody:[[NSDictionary dictionaryWithObject:responseDictionary forKey:@"com.AirFloat.PairingContainer"] dmapTaggedData]];
            [request.response setKeepAlive:YES];
            
            return;
            
        }
        
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.delegate pairerClientWasRejected:self];
    });
    
}

- (void)connectionDidClose:(AirFloatWebConnection *)connection {

    if (_loginToken)
        dispatch_async(dispatch_get_main_queue(), ^{
            [NSDefaultNotificationCenter postNotificationName:AirFloatDAAPPairerDidPairNotification object:self userInfo:[NSDictionary dictionaryWithObject:_loginToken forKey:kAirFloatDAAPPairerGuidKey]];
            [self.delegate pairerDidAuthenticate:self];
        });
    
}

@end

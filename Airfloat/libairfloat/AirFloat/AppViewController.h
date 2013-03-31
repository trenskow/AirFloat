//
//  AppViewController.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/11/13.
//
//

#import <UIKit/UIKit.h>

#import "raopserver.h"

@interface AppViewController : UIViewController

@property (assign,nonatomic,readwrite) raop_server_p server;

@end

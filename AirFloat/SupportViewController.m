//
//  SupportViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 3/20/13.
//
//

#import "SupportViewController.h"

@interface SupportViewController ()

@property (nonatomic, strong) IBOutlet UIWebView* webView;

@end

@implementation SupportViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    [self.webView loadHTMLString:[NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"Support"
                                                                                                    ofType:@"html"]
                                                           encoding:NSUTF8StringEncoding
                                                              error:nil]
                         baseURL:[[NSBundle mainBundle] bundleURL]];
    
    for (UIView *view in [[[self.webView subviews] objectAtIndex:0] subviews])
        if([view isKindOfClass:[UIImageView class]])
            [view removeFromSuperview];
    
    if ([self.webView respondsToSelector:@selector(scrollView)])
        self.webView.scrollView.decelerationRate = UIScrollViewDecelerationRateNormal;
    
    self.webView.alpha = 0.0;
    
}

- (void)webViewDidFinishLoad:(UIWebView *)webView {
    
    [UIView animateWithDuration:0.5
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.webView.alpha = 1.0;
                     } completion:nil];
    
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
    
    if (navigationType == UIWebViewNavigationTypeLinkClicked || navigationType == UIWebViewNavigationTypeFormSubmitted) {
        [[UIApplication sharedApplication] openURL:[request URL]];
        return NO;
    }
    
    return YES;
    
}

@end

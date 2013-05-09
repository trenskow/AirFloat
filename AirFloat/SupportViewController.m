//
//  SupportViewController.m
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    
    if ([self.webView respondsToSelector:@selector(scrollView)]) {
        self.webView.scrollView.decelerationRate = UIScrollViewDecelerationRateNormal;
        self.webView.scrollView.showsVerticalScrollIndicator = ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone);
    }
    
    self.webView.alpha = 0.0;
    
    [self centerContent];
    
}

- (CGFloat)heightOfContentInScrollView:(UIScrollView *)scrollView {
    
    NSString* documentHeight = [self.webView stringByEvaluatingJavaScriptFromString:@"document.getElementById('contentholder').getBoundingClientRect().top + document.getElementById('contentholder').getBoundingClientRect().height"];
    
    return [documentHeight doubleValue];
    
}

- (void)webViewDidFinishLoad:(UIWebView *)webView {
    
    [UIView animateWithDuration:0.5
                          delay:0.0
                        options:UIViewAnimationOptionCurveEaseOut
                     animations:^{
                         self.webView.alpha = 1.0;
                     } completion:nil];
    
    [self centerContent];
    
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
    
    if (navigationType == UIWebViewNavigationTypeLinkClicked || navigationType == UIWebViewNavigationTypeFormSubmitted) {
        [[UIApplication sharedApplication] openURL:[request URL]];
        return NO;
    }
    
    return YES;
    
}

@end

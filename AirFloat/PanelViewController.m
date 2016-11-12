//
//  PanelViewController.m
//  AirFloat
//
//  Created by Kristian Trenskow on 5/7/13.
//
//

#import "UIView+AirFloatAdditions.h"

#import "PanelViewController.h"

static int PanelViewControllerObserverContext;

@interface PanelViewController ()

- (void)keyboardWillAppearNotification:(NSNotification*)notification;
- (void)keyboardWillDisappearNotification:(NSNotification*)notification;

@end

@implementation PanelViewController

- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        
        [self.view addObserver:self forKeyPath:@"frame" options:0 context:&PanelViewControllerObserverContext];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillAppearNotification:) name:UIKeyboardWillShowNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillDisappearNotification:) name:UIKeyboardWillHideNotification object:nil];
        
    }
    
}

- (void)dealloc {
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        
        [self.view removeObserver:self forKeyPath:@"frame" context:&PanelViewControllerObserverContext];
        
    }
    
}

- (UIScrollView*)findScrollViewInView:(UIView *)view {
    
    if ([view isKindOfClass:[UIScrollView class]])
        return (UIScrollView*)view;
    
    for (UIView* subview in view.subviews) {
        UIScrollView* ret;
        if ((ret = [self findScrollViewInView:subview]) != nil)
            return ret;
    }
    
    return nil;
    
}

- (CGFloat)heightOfContentInScrollView:(UIScrollView *)scrollView {
    
    return ((UIView*)[scrollView.subviews objectAtIndex:0]).frame.size.height;
    
}

- (void)centerContent {
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad) {
        
        UIScrollView* scrollView = [self findScrollViewInView:self.view];
        
        CGFloat height = [self heightOfContentInScrollView:scrollView];
        
        scrollView.contentInset = UIEdgeInsetsMake((scrollView.frame.size.height - height) / 2.0,
                                                   0, 0, 0);
        scrollView.contentSize = CGSizeMake(scrollView.bounds.size.width, height);
        
    }
    
}

- (void)keyboardWillAppearNotification:(NSNotification *)notification {
    
    CGRect endFrame = [[[notification userInfo] objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    
    endFrame = [self.view.window convertRect:endFrame toView:self.view];
    
    endFrame = CGRectIntersection(endFrame, self.view.bounds);
    
    CGPoint center = self.view.center;
    center.y -= round(endFrame.size.height / 2);
    
    [UIView beginAnimations:@"offset" context:nil];
    [UIView setAnimationCurve:[[[notification userInfo] objectForKey:UIKeyboardAnimationCurveUserInfoKey] integerValue]];
    [UIView setAnimationDuration:[[[notification userInfo] objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue]];
    
    self.view.center = center;
    
    [UIView commitAnimations];
    
}

- (void)keyboardWillDisappearNotification:(NSNotification *)notification {
    
    [UIView beginAnimations:@"offset" context:nil];
    [UIView setAnimationCurve:[[[notification userInfo] objectForKey:UIKeyboardAnimationCurveUserInfoKey] integerValue]];
    [UIView setAnimationDuration:[[[notification userInfo] objectForKey:UIKeyboardAnimationDurationUserInfoKey] doubleValue]];
    
    self.view.center = CGPointMake(self.view.bounds.size.width / 2,
                                   self.view.bounds.size.height / 2);
    
    [UIView commitAnimations];
    
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    
    if (context == &PanelViewControllerObserverContext)
        [self centerContent];
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    
}

@end

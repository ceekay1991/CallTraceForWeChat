//
//  BBASMCallTraceProfilerEntrance.m
//  BBAMNP
//
//  Created by chenronghang on 2020/2/17.
//  Copyright © 2020 Baidu. All rights reserved.
//

#import "BBASMCallTraceProfilerEntrance.h"
#import "BBASMCallTraceInterface.h"
#import "BBASMTimeProfilerVC.h"
#import "BBASMCallTraceRunTime.h"
#import <objc/runtime.h>
#import <objc/message.h>

@implementation BBASMCallTraceProfilerEntrance

#ifdef BBASMCallTraceEnable
+ (void)load {
    [self swizzleMethods];
}
#endif

+ (void)swizzleMethods {
    [self addShowCallTraceViewEntrance];
    
    //打开小程序
    [BBASMCallTraceRunTime replaceClass:NSClassFromString(@"WAAppContactPreLoader")
                                    sel:NSSelectorFromString(@"openApp:taskExtInfo:onSuccess:onFailed:")
                              withClass:[self class]
                                withSEL:@selector(_openApp:taskExtInfo:onSuccess:onFailed:)];
    
    //小程序加载完成
    [BBASMCallTraceRunTime replaceClass:NSClassFromString(@"WAAppTask")
                                    sel:NSSelectorFromString(@"taskDidOpen")
                              withClass:[self class]
                                withSEL:@selector(_taskDidOpen)];
    //小程序关闭
    [BBASMCallTraceRunTime replaceClass:NSClassFromString(@"WAWebViewController")
                                    sel:NSSelectorFromString(@"onAllExit")
                              withClass:[self class]
                                withSEL:@selector(_taskDidOpen)];
    [BBASMCallTraceRunTime replaceClass:NSClassFromString(@"WAJSCoreService")
                                    sel:NSSelectorFromString(@"setThread:")
                              withClass:[self class]
                                withSEL:@selector(_setThread:)];
}



+ (void)addShowCallTraceViewEntrance {
    Class cls = NSClassFromString(@"WAAppAuthorizationViewController");
    IMP mainimp = [self instanceMethodForSelector:@selector(showMainThreadCallTraceView:)];
    class_addMethod(cls, @selector(showMainThreadCallTraceView:), mainimp, "v@:");
    
    IMP jsimp = [self instanceMethodForSelector:@selector(showJSThreadCallTraceView:)];
    class_addMethod(cls, @selector(showJSThreadCallTraceView:), jsimp, "v@:");
    
    [BBASMCallTraceRunTime replaceClass:cls
                                    sel:NSSelectorFromString(@"viewDidLoad")
                              withClass:[self class]
                                withSEL:@selector(_viewDidLoad)];
}

- (void)_openApp:(id)arg1
     taskExtInfo:(id)arg2
       onSuccess:(id)arg3
        onFailed:(id)arg4 {
    [BBASMCallTraceProfilerEntrance startCallTrace];
    [self _openApp:arg1 taskExtInfo:arg2 onSuccess:arg3 onFailed:arg4];
}

- (void)_taskDidOpen {
    [self _taskDidOpen];
    [BBASMCallTrace stopTrace];
}

- (void)_onAllExit{
    [BBASMCallTrace clearTrace];
    [self _onAllExit];
}

- (void)_setThread:(NSThread *)thread {
    if (thread && !thread.name) {
        thread.name = @"recordThreadByTrace";
    }
    
    [self _setThread:thread];
}

- (void)_viewDidLoad {
    [self _viewDidLoad];
    UIViewController *viewController = (UIViewController *)self;
    
    UIView *view = [[viewController navigationController] navigationBar];
    CGFloat buttonH = 40;
    CGFloat buttonW = 100;
    CGFloat margin = 5;
    CGRect frame = CGRectMake(40, (CGRectGetHeight(view.frame)-buttonH)/2.0f, buttonW, buttonH);
    UIButton *button = [BBASMCallTraceProfilerEntrance buttonWithTitle:@"trace-主" frame:frame];
    [button addTarget:self action:@selector(showMainThreadCallTraceView:) forControlEvents:UIControlEventTouchUpInside];
    [view addSubview:button];
    
    CGFloat x = CGRectGetMaxX(frame) + margin * 4;
    frame.origin.x = x;
    UIButton *jsButton = [BBASMCallTraceProfilerEntrance buttonWithTitle:@"trace-js" frame:frame];
    [jsButton addTarget:self action:@selector(showJSThreadCallTraceView:) forControlEvents:UIControlEventTouchUpInside];
    [view addSubview:jsButton];
}

+ (UIButton *)buttonWithTitle:(NSString *)title
                        frame:(CGRect)frame {
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    button.frame = frame;
    [button setTitle:title forState:UIControlStateNormal];
    //#3388ff
    [button setBackgroundColor:[UIColor colorWithRed:51.0f/255.0f green:136.0f/255.0f blue:1.0f alpha:1]];
    button.layer.cornerRadius = 2;
    return button;
}

- (void)showMainThreadCallTraceView:(UIButton *)sender {
    [BBASMCallTraceProfilerEntrance showTimeProfile:[BBASMCallTrace getMainThreadCallRecord]
                                          currentVC:self];
}

- (void)showJSThreadCallTraceView:(UIButton *)sender {
    [BBASMCallTraceProfilerEntrance showTimeProfile:[BBASMCallTrace getOtherThreadCallRecord]
                                          currentVC:self];
}

+ (void)showTimeProfile:(BBASMThreadCallRecord *)record currentVC:(UIViewController *)vc{
    if (BBASMTimeProfilerVCHasShow) {
        return;
    }
    if (!record) {
        NSLog(@"无记录");
        return;
    }
    BBASMTimeProfilerVC *profilerVC = [[BBASMTimeProfilerVC alloc] initWithCallRecord:record];
    profilerVC.modalPresentationStyle = UIModalPresentationFullScreen;
    [vc presentViewController:profilerVC animated:YES completion:nil];
}

+ (void)startCallTrace {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [BBASMCallTrace setCostMinTime:500];
        [BBASMCallTrace setMaxDepth:100];
        [BBASMCallTrace setMonitorThreadName:"recordThreadByTrace"];
    });
    [BBASMCallTrace startTrace];
}

@end

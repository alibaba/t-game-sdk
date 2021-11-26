/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#import "TBJAppController.h"
#import "TBJViewController.h"
#include "TBJAppFrameworkInternal.h"

@interface TBJAppController() {
}

@property(nonatomic, readwrite) TBJViewController* viewController;
@end


@implementation TBJAppController

@synthesize window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    window = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
    self.viewController = [[TBJViewController alloc] init];
    self.viewController.wantsFullScreenLayout = YES;
    if ( [[UIDevice currentDevice].systemVersion floatValue] < 6.0)
    {
        [window addSubview: self.viewController.view];
    }
    else
    {
        [window setRootViewController:self.viewController];
    }
    
    [window makeKeyAndVisible];
    [[UIApplication sharedApplication] setStatusBarHidden:YES];
    if (tbjAppMainCallBack != NULL)
    {
        tbjAppMainCallBack(tbjMainArgc, tbjMainArgv);
    }
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
}

- (void)applicationWillTerminate:(UIApplication *)application {
}

@end

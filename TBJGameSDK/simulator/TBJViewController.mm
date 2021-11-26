/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#import "TBJViewController.h"
#import "TBJGameView.h"

@implementation TBJViewController

- (instancetype)init
{
    self = [super init];
    self.view = [[TBJGameView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
}

- (BOOL) shouldAutorotate {
    return YES;
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
    [super didRotateFromInterfaceOrientation:fromInterfaceOrientation];
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}


@end

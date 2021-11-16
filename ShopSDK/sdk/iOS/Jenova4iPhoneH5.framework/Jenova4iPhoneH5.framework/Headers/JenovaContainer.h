/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef JenovaContainer_h
#define JenovaContainer_h

#import <UIKit/UIKit.h>
#import <Jenova4iPhoneH5/JenovaCommon.h>

@interface JenovaContainer : NSObject

- (instancetype) init __attribute__((unavailable("use initWithBizId")));
- (instancetype) initWithBizId: (NSString*) bizId;
- (void) setOrientation:(JenovaContainerOrientation)Orientation;
- (void) show: (UIViewController*)currentVC userID:(NSString*) userID itemID:(NSString*)itemID;

- (void) userLogout;

@end


#endif /* JenovaContainer_h */

/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface TBJGameView : UIView <UIKeyInput, UITextInput, UITextInputTraits>
{
}

-(instancetype)initWithFrame:(CGRect)rect;

-(void)surfaceChanged;

-(void)changeVisible:(BOOL)visible;

-(void)swapBuffer;

-(int)frameWidth;

-(int)frameHeight;

-(void)start;

-(void)finishSelf;

-(bool)isKeyboardShowing;

-(void)showKeyboard:(int)type;

-(void)hideKeyboard;

-(const char*)getEditContent;

@end

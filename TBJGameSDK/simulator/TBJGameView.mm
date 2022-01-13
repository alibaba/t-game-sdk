/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#import <OpenGLES/ES3/gl.h>
#import "TBJGameView.h"
#import "TBJEnvContext.h"
#import "TBJStorageContext.h"
#import "TBJGameInstance.h"
#import "TBJAppFrameworkInternal.h"

@interface WasmGameGestureRecognizer : UIGestureRecognizer
{
}

@property(nonatomic, weak) TBJGameView* gameView;

@end

@interface TBJGameView() {
    TBJGameInstance* _gameInstance;
    CFTimeInterval lastFrameTime;
    
    volatile int _frameWidth;
    volatile int _frameHeight;
    CAEAGLLayer* eaglLayer;
    
    volatile int _fps;
    int _fpsFrames;
    CFTimeInterval _fpsStartTime;
    bool enableFPSInfo;
    UITextField* _fpsView;
    char    inputText[256];
    volatile bool isKeyboardShowing;
}

@property(nonatomic, strong) UITextField* uiTextInputDelegate;
@property(nonatomic, strong) CADisplayLink* gameDisplayLink;
@property(nonatomic, strong) EAGLContext* glContext;
@property(nonatomic, strong) WasmGameGestureRecognizer* gestureRecognizer;

- (void)touchesBeganCustom:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
- (void)touchesMovedCustom:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
- (void)touchesEndedCustom:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
- (void)touchesCancelledCustom:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;

@end


@implementation WasmGameGestureRecognizer

- (instancetype)initWithTarget:(nullable id)target action:(nullable SEL)action
{
    self = [super initWithTarget:target action:action];
    return self;
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [super touchesBegan:touches withEvent:event];
    
    if (self.gameView != nil)
    {
        [self.gameView touchesBeganCustom:touches withEvent:event];
    }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [super touchesMoved:touches withEvent:event];
    
    if (self.gameView != nil)
    {
        [self.gameView touchesMovedCustom:touches withEvent:event];
    }
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [super touchesEnded:touches withEvent:event];
    if (self.gameView != nil)
    {
        [self.gameView touchesEndedCustom:touches withEvent:event];
    }
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    [super touchesCancelled:touches withEvent:event];
    if (self.gameView != nil)
    {
        [self.gameView touchesCancelledCustom:touches withEvent:event];
    }
}
@end

@implementation TBJGameView

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    [self setBackgroundColor:[UIColor whiteColor]];
    [self setMultipleTouchEnabled:YES];
    
    self.uiTextInputDelegate = nil;
    self.glContext = nil;
    self.gestureRecognizer = [[WasmGameGestureRecognizer alloc] initWithTarget:nil action:nil];
    self.gestureRecognizer.gameView = self;
    [self addGestureRecognizer:self.gestureRecognizer];
    
    isKeyboardShowing = false;
    
    enableFPSInfo = true;
    [self resetFPS];
    _fpsView = nil;
    
    [self setContentScaleFactor:[[UIScreen mainScreen] scale]];
    const char* appName = "DEFAULT_GAME_NAME";
    
    NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *docDir = [paths objectAtIndex:0];
    NSString* appPath = [NSString stringWithFormat: @"%@/wasmgame_container/%s", docDir, appName];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    if(![fileManager fileExistsAtPath:appPath]) {
        [fileManager createDirectoryAtPath:appPath withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    eaglLayer = (CAEAGLLayer*)self.layer;
    
    [self fetchFrameSize];
    if (enableFPSInfo)
    {
        CGRect rect;
        rect.origin.x = rect.origin.y = 0;
        rect.size.width = 100; rect.size.height = 32;
        _fpsView = [[UITextField alloc] initWithFrame:rect];
        [_fpsView setEnabled:FALSE];
        [self addSubview:_fpsView];
    }
    
    if ([bundlePath characterAtIndex:([bundlePath length] - 1)] != '/')
    {
        bundlePath = [bundlePath stringByAppendingString:@"/"];
    }
    if ([appPath characterAtIndex:([appPath length] - 1)] != '/')
    {
        appPath = [appPath stringByAppendingString:@"/"];
    }
    
    TBJEnvContext* envContext = new TBJEnvContext();
    envContext->setNativeContext((__bridge void*)self);
    envContext->setGameRootPath([bundlePath UTF8String]);
    envContext->setGameWritePath([appPath UTF8String]);
    _gameInstance = new TBJGameInstance();
    _gameInstance->setEnvContext(envContext);
    
    std::string storageRoot = std::string(envContext->getGameWritePath()) + std::string("/wasmgame_storage.db");
    TBJStorageContext* storageContext = new TBJStorageContext(storageRoot.c_str());
    _gameInstance->setStorageContext(storageContext);
    
    [self initSurface];
    [self startLoop];
    return self;
}

- (void)resetFPS
{
    _fps = -1;
    _fpsStartTime = -1;
    _fpsFrames = 0;
}

- (int)getFPS
{
    return _fps;
}

- (void)addFrame:(CFTimeInterval)time
{
    if (_fpsStartTime < 0) _fpsStartTime = time;
    else _fpsFrames++;
    
    CFTimeInterval delta = time - _fpsStartTime;
    if (delta > 1.0)
    {
        _fps = (int)(_fpsFrames / delta);
        _fpsFrames = 0;
        _fpsStartTime = time;
    }
}

- (void) layoutSubviews
{
    [self fetchFrameSize];
    
    if (_gameInstance != nullptr)
    {
        _gameInstance->resizeBuffer();
    }
}

-(void)surfaceChanged
{
    [self fetchFrameSize];
    
    if (_gameInstance != nullptr)
    {
        _gameInstance->resizeBuffer();
    }
}

- (void)startLoop
{
    [self fetchFrameSize];
    eaglLayer = (CAEAGLLayer*)self.layer;
    
    self.gameDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(gameThreadRenderFrame)];
    lastFrameTime = [self.gameDisplayLink timestamp];
    if (@available(iOS 10.0, *)) {
        [self.gameDisplayLink setPreferredFramesPerSecond:60];
    }
    else{
        [self.gameDisplayLink setFrameInterval:60];
    }
    
    [self.gameDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
}

//////////////// game thread function start ///////////////////////
- (void) initSurface
{
    const char* driver = _gameInstance->getEnvContext()->getString("driver");
    if (driver == NULL || strcmp(driver, "gl") == 0 || strcmp(driver, "gles") == 0 || strcmp(driver, "opengl") == 0 || strcmp(driver, "opengles") == 0)
    {
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        int glVersion = _gameInstance->getEnvContext()->getInt("glversion");
        if (glVersion == 0) glVersion = 3;
        
        EAGLRenderingAPI glapiv = (glVersion == 3) ? kEAGLRenderingAPIOpenGLES3 : (glVersion == 2 ? kEAGLRenderingAPIOpenGLES2 : kEAGLRenderingAPIOpenGLES1);
        self.glContext = [[EAGLContext alloc] initWithAPI:glapiv];
        [EAGLContext setCurrentContext:self.glContext];
        
        _gameInstance->initGLESFrameBuffer(glVersion);
        if(![self.glContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer])
        {
            NSLog(@"failed to call context");
        }
        _gameInstance->resizeBuffer();
    } else
    {
    }
}

-(void)destroy_gameThread
{
    if (_gameInstance != nil)
    {
        delete _gameInstance;
        _gameInstance = nil;
    }
    [EAGLContext setCurrentContext:nil];
    
    self.glContext = nil;
    
    [[NSThread currentThread] cancel];
    
    if (self.gameDisplayLink)
    {
        [self.gameDisplayLink invalidate];
        self.gameDisplayLink = nil;
    }
    
    if (self.uiTextInputDelegate != nil)
    {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardDidShowNotification object:nil];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardWillHideNotification object:nil];
        
        [self.uiTextInputDelegate removeFromSuperview];
        self.uiTextInputDelegate = nil;
    }
    
    if (_fpsView != nil)
    {
        [_fpsView removeFromSuperview];
        _fpsView = nil;
    }
    
    if (self.gestureRecognizer)
    {
        [self removeGestureRecognizer:self.gestureRecognizer];
        self.gestureRecognizer.gameView = nil;
        self.gestureRecognizer = nil;
    }
}

- (void)gameThreadRenderFrame
{
    if (self.glContext != nil)
    {
        if (_gameInstance->isGameShouldClose())
        {
            [self destroy_gameThread];
            return;
        }
        
        if (self.glContext != [EAGLContext currentContext])
            glFlush();

        if (![EAGLContext setCurrentContext: self.glContext])
        {
            NSLog(@"Make Current Context Failed\n");
        }
        
        CFTimeInterval currentTime = [self.gameDisplayLink timestamp];
        CFTimeInterval deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        _gameInstance->gameLoopOnce((float)deltaTime);
        
        if (enableFPSInfo)
        {
            [self addFrame:currentTime];
        }
        
        if (_fps >= 0)
        {
            NSString *nsfps = [NSString stringWithFormat:@"FPS: %d", _fps];
            [_fpsView setText:nsfps];
        }
    }
}

-(void)swapBuffer
{
    if (self.glContext != NULL)
    {
//        printf("self.glContext retain count %d\n", CFGetRetainCount((__bridge CFTypeRef)(self.glContext)));
        if(![self.glContext presentRenderbuffer:GL_RENDERBUFFER])
        {
            NSLog(@"GLContext Swap Buffer failed");
        }
    }
}

////////////////////////// key board /////////////////////////
-(void)showKeyboard:(int)type
{
    if (self.uiTextInputDelegate == nil)
    {
        CGRect rect;
        rect.origin.x = 0;
        rect.origin.y = 0;
        rect.size.width = self.bounds.size.width;
        rect.size.height = 48;
        self.uiTextInputDelegate = [[UITextField alloc] initWithFrame:rect];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onUIKeyboardNotification:)
                                                     name:UIKeyboardDidShowNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onUIKeyboardNotification:)
                                                     name:UIKeyboardWillHideNotification object:nil];
    }
    
    CGRect rect;
    rect.origin.x = 0;
    rect.origin.y = 0;
    rect.size.width = self.bounds.size.width;
    rect.size.height = 48;
    [self.uiTextInputDelegate setFrame:rect];
    [self.uiTextInputDelegate setText:@""];
    [self addSubview:self.uiTextInputDelegate];
    [self.uiTextInputDelegate setAutocapitalizationType:(UITextAutocapitalizationType)type];
    [self.uiTextInputDelegate becomeFirstResponder];
}

-(bool)isKeyboardShowing
{
    return isKeyboardShowing;
}

-(void)hideKeyboard
{
    if (self.uiTextInputDelegate != nil)
    {
        [self.uiTextInputDelegate removeFromSuperview];
        [self.uiTextInputDelegate resignFirstResponder];
    }
}

-(const char*)getEditContent
{
    if (self.uiTextInputDelegate != nil)
    {
        return [[self.uiTextInputDelegate text] UTF8String];
    }
    return nullptr;
}

- (void)onUIKeyboardNotification:(NSNotification *)notif
{
    NSString * type = notif.name;
    
    NSDictionary* info = [notif userInfo];
    CGRect end = [self convertRect:
                  [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue]
                          fromView:self];
    if (type == UIKeyboardDidShowNotification)
    {
        isKeyboardShowing = true;
        CGRect rect;
        rect.origin.x = 0;
        rect.origin.y = end.origin.y - 48;
        rect.size.width = self.bounds.size.width;
        rect.size.height = 48;
        [self.uiTextInputDelegate setFrame:rect];
    } else if (type == UIKeyboardWillHideNotification)
    {
        isKeyboardShowing = false;
        NSString* text = [self.uiTextInputDelegate text];
        [self.uiTextInputDelegate removeFromSuperview];
        
        TBJGameEvent gameEvent;
        gameEvent.eventType = TAOBAO_KEYBOARD_EVENT;
        gameEvent.keyboard.keyboardType = TAOBAO_MOBILE_KEYBOARD_INSERT;
        
        int length = (int)([text length]);
        length = length < 255 ? length : 255;
        memcpy(inputText, [text UTF8String], length);
        inputText[length] = 0;
        gameEvent.keyboard.inputText = inputText;
        _gameInstance->invokeGameEvent(gameEvent);
    }
}

/////////////////////// game thread functions end //////////////
///
-(void)fetchFrameSize
{
    CGSize bound = [self bounds].size;
    _frameWidth = (int)bound.width * self.contentScaleFactor;
    _frameHeight = (int)bound.height * self.contentScaleFactor;
}

-(int)frameWidth
{
    return _frameWidth;
}

-(int)frameHeight
{
    return _frameHeight;
}

#pragma mark Touch
- (BOOL)canBecomeFirstResponder
{
    return YES;
}

- (void)processTouch:(UITouch*) touch withEvent:(TBJGameEvent&)gameEvent
{
    intptr_t touchId = (intptr_t)(__bridge void*)touch;
    gameEvent.touch.x = [touch locationInView: [touch view]].x * self.contentScaleFactor;
    gameEvent.touch.y = [touch locationInView: [touch view]].y * self.contentScaleFactor;
    gameEvent.touch.previousX = [touch previousLocationInView:[touch view]].x * self.contentScaleFactor;
    gameEvent.touch.previousY = [touch previousLocationInView:[touch view]].y * self.contentScaleFactor;
    gameEvent.touch.tapCount = [touch tapCount];
    
    gameEvent.eventType = TAOBAO_TOUCH_EVENT;
    if (NSFoundationVersionNumber >= NSFoundationVersionNumber_iOS_9_0)
    {
        gameEvent.touch.pressure = [touch force];
    } else
    {
        gameEvent.touch.pressure = 1.0f;
    }
    
    long long touchIdL = (long long)touchId;
    gameEvent.touch.touchId = (int)touchIdL;
    
    _gameInstance->invokeGameEvent(gameEvent);
}

- (void)touchesBeganCustom:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_gameInstance == nil) return;
    
    TBJGameEvent gameEvent;
    for (UITouch* touch in touches)
    {
        gameEvent.touch.touchType = TAOBAO_TOUCH_DOWN;
        [self processTouch:touch withEvent:gameEvent];
    }
}

- (void)touchesMovedCustom:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_gameInstance == nil) return;
    
    TBJGameEvent gameEvent;
    for (UITouch* touch in touches)
    {
        gameEvent.touch.touchType = TAOBAO_TOUCH_MOVE;
        [self processTouch:touch withEvent:gameEvent];
    }
}

- (void)touchesEndedCustom:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_gameInstance == nil) return;
    
    TBJGameEvent gameEvent;
    for (UITouch* touch in touches)
    {
        gameEvent.touch.touchType = TAOBAO_TOUCH_UP;
        [self processTouch:touch withEvent:gameEvent];
    }
}
    
- (void)touchesCancelledCustom:(NSSet *)touches withEvent:(UIEvent *)event
{
    if (_gameInstance == nil) return;
    
    TBJGameEvent gameEvent;
    for (UITouch* touch in touches)
    {
        gameEvent.touch.touchType = TAOBAO_TOUCH_UP;
        [self processTouch:touch withEvent:gameEvent];
    }
}

@end

/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#import <UIKit/UIKit.h>
#import "TBJGameEnviroment.h"
#import "TBJAppController.h"
#import "TBJGameView.h"
#include "TBJEnvContext.h"
#include "TBJStorageContext.h"
#include "TBJGameInstance.h"

TBJGameInstance* tbjGlobalInstance = nullptr;
TBJGameNativeMainCallBack tbjAppMainCallBack = nullptr;
int tbjMainArgc = 0;
char** tbjMainArgv = nullptr;
char tbjGlobalConfig[512] = {0};

unsigned long long TBJGameCreateSurface(const char* name, int width, int height)
{
    return 1L;
}


int TBJGameNativeMain(TBJGameNativeMainCallBack callBack, int argc, char* argv[])
{
    tbjAppMainCallBack = callBack;
    tbjMainArgc = argc;
    tbjMainArgv = (char**)malloc(sizeof(char*) * argc);
    for (int i = 0; i < argc; i++)
    {
        tbjMainArgv[i] = argv[i];
    }
    if (tbjAppMainCallBack == NULL)
    {
        printf("has no native call\n");
        return 1;
    }
    return UIApplicationMain(1, argv, nil, @"TBJAppController");
}

void TBJGameNativeConfig(const char* content)
{
    memcpy(tbjGlobalConfig, content, strlen(content));
    tbjGlobalConfig[strlen(content)] = 0;
}

void TBJGameSetGameLoop(TBJGameLoop gameLoop)
{
    tbjGlobalInstance->setGameLoop(gameLoop);
}

void TBJGameSetEventListener(TBJGameEventListener eventListener)
{
    tbjGlobalInstance->setEventListener(eventListener);
}

void TBJGameExit()
{
    tbjGlobalInstance->setGameShouldClose(true);
}

void TBJGameSwapBuffers(unsigned long long surfaceId)
{
    TBJGameView* gameView = (__bridge TBJGameView*)(tbjGlobalInstance->getEnvContext()->getNativeContext());
    if (gameView != NULL)
    {
        [gameView swapBuffer];
    }
}

int TBJGameGetDefaultFrameBuffer(unsigned long long surfaceId)
{
    return tbjGlobalInstance->getDefaultFrameBuffer();
}
int TBJGameGetDefaultColorBuffer(unsigned long long surfaceId)
{
    return tbjGlobalInstance->getDefaultColorBuffer();
}
int TBJGameGetDefaultDepthBuffer(unsigned long long surfaceId)
{
    return tbjGlobalInstance->getDefaultDepthBuffer();
}

int TBJGameGetFrameWidth(unsigned long long surfaceId)
{
    TBJGameView* gameView = (__bridge TBJGameView*)(tbjGlobalInstance->getEnvContext()->getNativeContext());
    if (gameView != NULL)
    {
        return [gameView frameWidth];
    }
    return 512;
}

int TBJGameGetFrameHeight(unsigned long long surfaceId)
{
    TBJGameView* gameView = (__bridge TBJGameView*)(tbjGlobalInstance->getEnvContext()->getNativeContext());
    if (gameView != NULL)
    {
        return [gameView frameHeight];
    }
    return 512;
}

void TBJGameSetFrameSize(unsigned long long surfaceId, int width, int height)
{
}

bool TBJGameIsKeyboardShowing(unsigned long long surfaceId)
{
    TBJGameView* gameView = (__bridge TBJGameView*)(tbjGlobalInstance->getEnvContext()->getNativeContext());
    if (gameView != NULL)
    {
        return [gameView isKeyboardShowing];
    }
    return false;
}
void TBJGameShowKeyboard(unsigned long long surfaceId, int type)
{
    TBJGameView* gameView = (__bridge TBJGameView*)(tbjGlobalInstance->getEnvContext()->getNativeContext());
    if (gameView != NULL)
    {
        [gameView showKeyboard:type];
    }
}

void TBJGameHideKeyboard(unsigned long long surfaceId)
{
    TBJGameView* gameView = (__bridge TBJGameView*)(tbjGlobalInstance->getEnvContext()->getNativeContext());
    if (gameView != NULL)
    {
        [gameView hideKeyboard];
    }
}

const char* TBJGameGetBundleRoot(unsigned long long surfaceId)
{
    return tbjGlobalInstance->getEnvContext()->getGameRootPath();
}

const char* TBJGameGetStorageRoot(unsigned long long surfaceId)
{
    return tbjGlobalInstance->getEnvContext()->getGameWritePath();
}

void TBJGameCopyToClipboard(const char* msg)
{
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    pasteboard.string = [[NSString alloc] initWithUTF8String:msg];
}

const char* TBJGameGetClipboardContent()
{
    UIPasteboard *pasteboard = [UIPasteboard generalPasteboard];
    return [pasteboard.string UTF8String];
}

int TBJGameGetDeviceBulgeHeaderHeight()
{
    return 0;
}

void TBJGameStoreKeyValue(unsigned long long surfaceId, const char* key, const char* value)
{
    tbjGlobalInstance->getStorageContext()->putValue(key, value);
}

void TBJGameRemoveStoreKey(unsigned long long surfaceId, const char* key)
{
    tbjGlobalInstance->getStorageContext()->removeKey(key);
}

const char* TBJGameGetStoreValue(unsigned long long surfaceId, const char* key)
{
    tbjGlobalInstance->getStorageContext()->getValue(key);
}
void TBJGameClearStore(unsigned long long surfaceId)
{
    tbjGlobalInstance->getStorageContext()->clear();
}

int TBJGameGetPathStatus(const char* path)
{
    if (path == nullptr) return 0;
    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDirectory = NO;
    if ([fm fileExistsAtPath:[NSString stringWithUTF8String:path] isDirectory:&isDirectory])
    {
        if (isDirectory) return 2;
        else return 1;
    } else
    {
        return 0;
    }
}

int TBJGameCreateDirectory(const char* dir)
{
    if (dir == nullptr) return 0;
    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDirectory = NO;
    if ([fm fileExistsAtPath:[NSString stringWithUTF8String:dir] isDirectory:&isDirectory])
    {
        if (isDirectory) return 1;
        else return 0;
    }
    BOOL result = [fm createDirectoryAtPath:[NSString stringWithUTF8String:dir] withIntermediateDirectories:YES attributes:nil error:nil];
    return result ? 1 : 0;
}

int TBJGameDeletePath(const char* path)
{
    if (path == nullptr) return 1;
    NSFileManager* fm = [NSFileManager defaultManager];
    
    BOOL isDirectory = NO;
    if ([fm fileExistsAtPath:[NSString stringWithUTF8String:path] isDirectory:&isDirectory])
    {
        NSError* error = NULL;
        [fm removeItemAtPath:[NSString stringWithUTF8String:path] error:&error];
        if (error == NULL) return 1;
        return 0;
    }
    else
    {
        return 1;
    }
}

int TBJGameCopyPath(const char* fromPath, const char* toPath)
{
    if (fromPath == nullptr || toPath == nullptr) return 1;
    NSFileManager* fm = [NSFileManager defaultManager];
    
    BOOL isDirectory = NO;
    if ([fm fileExistsAtPath:[NSString stringWithUTF8String:fromPath] isDirectory:&isDirectory])
    {
        NSError* error = NULL;
        [fm copyItemAtPath:[NSString stringWithUTF8String:fromPath] toPath:[NSString stringWithUTF8String:toPath] error:&error];
        if (error == NULL) return 1;
        return 0;
    }
    else
    {
        return 1;
    }
}

void TBJGameGenerateUUID(char result[37])
{
    CFUUIDRef theUUID = CFUUIDCreate(NULL);
    CFStringRef string = CFUUIDCreateString(NULL, theUUID);
    CFRelease(theUUID);
    const char* nativeStr = [((__bridge NSString *)string) UTF8String];
    int length = (int)strlen(nativeStr);
    length = std::min(length, 36);
    memcpy(result, nativeStr, length);
    result[length] = 0;
}

unsigned int TBJGamePlayAudio(const char* filePath, int loop, float pitch, float pan, float gain)
{
    return 0;
}


void TBJGameStopAudio(unsigned int audioId, int fadeOut)
{
}

void glMapBufferRead(void *dst, const void *glBuffer, int n)
{
    memcpy(dst, glBuffer, n);
}
void glMapBufferWrite(void* glBuffer, const void* src, int n)
{
    memcpy(glBuffer, src, n);
}


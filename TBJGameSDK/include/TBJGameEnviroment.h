/*
 * Copyright (C) 2021-2014 Alibaba Group Holding Limited
 */

#ifndef _TAOBAO_JENOVA_ENVIROMENT_H_
#define _TAOBAO_JENOVA_ENVIROMENT_H_

#include "TBJGameMacro.h"

TAOBAO_NS_BEGIN

void glMapBufferRead(void *dst, const void *glBuffer, int n);
void glMapBufferWrite(void* glBuffer, const void* src, int n);

#define TAOBAO_KEYBOARD_EVENT 0
#define TAOBAO_TOUCH_EVENT 1
#define TAOBAO_ACCELERATE_EVENT 2
#define TAOBAO_FOCUS_EVENT 3
#define TAOBAO_SYSTEM_EVENT 4

#define TAOBAO_MOBILE_KEYBOARD_INSERT 0
#define TAOBAO_PC_KEYBOARD_DOWN 1
#define TAOBAO_PC_KEYBOARD_UP 2
#define TAOBAO_PC_KEYBOARD_PRESSING 3
typedef struct TBJGameKeyboardEvent
{
	int eventType;		// 0
	int keyboardType; // 0 : mobile text input. 1 : pc keyboard down. 2 : pc keyboard up. 3 : pc keyboard pressing
    union {
        int keyWord;
        const char* inputText;  // this ptr is reused among every mobile keyboard events.
    };
} TBJGameKeyboardEvent;

#define TAOBAO_TOUCH_DOWN 0
#define TAOBAO_TOUCH_UP 1
#define TAOBAO_TOUCH_MOVE 2

typedef struct TBJGameTouchEvent
{
	int eventType;	// 1
	int touchType; // 0 : touch down. 1 : touch up. 2 : touch move.
    int touchId;
    unsigned int tapCount;
	float x;
	float y;
    float previousX;
    float previousY;
	float pressure;
} TBJGameTouchEvent;

typedef struct TBJGameAccelerateEvent
{
	int eventType; // 2
	float dirx;
	float diry;
	float dirz;
	float aspeed;
	float speed;
} TBJGameAccelerateEvent;

typedef struct TBJGameGroundEvent
{
	int eventType; // 3;
	int groudEventType; // 0: enter backgroud.  1 : enter fowardground.
} TBJGameGroundEvent;

#define TAOBAO_SYSCALL_RESIZE 0
#define TAOBAO_SYSCALL_PREDESTROY 1
#define TAOBAO_SYSCALL_PAUSE 2
#define TAOBAO_SYSCALL_RESUME 3

typedef struct TBJGameSystemCallEvent
{
	int eventType; // 4;
	int systemCallType; // 0 : resize, 1: preDestroy, 2 : pause, 3 : resume
    union {
        struct {
            unsigned int width;
            unsigned int height;
        } frameSize;
    };
} TBJGameSystemCallEvent;

typedef union TBJGameEvent
{
    int eventType;    // 0 : keyboard. 1 : touch. 2 : accelerate. 3 : enter or leave. 4 . system call
    TBJGameKeyboardEvent keyboard;
    TBJGameTouchEvent touch;
    TBJGameAccelerateEvent accelerate;
    TBJGameGroundEvent ground;
    TBJGameSystemCallEvent systemCall;
} TBJGameEvent;

typedef void (*TBJGameLoop)(float time);
typedef void (*TBJGameEventListener)(const TBJGameEvent* evt);

// Call in main thread
void TBJGameSetGameLoop(TBJGameLoop gameLoop);
void TBJGameSetEventListener(TBJGameEventListener eventListener);
void TBJGameExit();
void TBJGameSwapBuffers(unsigned long long surfaceId);
int TBJGameGetDefaultFrameBuffer(unsigned long long surfaceId);
int TBJGameGetDefaultColorBuffer(unsigned long long surfaceId);
int TBJGameGetDefaultDepthBuffer(unsigned long long surfaceId);
int TBJGameGetFrameWidth(unsigned long long surfaceId);
int TBJGameGetFrameHeight(unsigned long long surfaceId);
void TBJGameSetFrameSize(unsigned long long surfaceId, int width, int height);
bool TBJGameIsKeyboardShowing(unsigned long long surfaceId);
void TBJGameShowKeyboard(unsigned long long surfaceId, int type);
void TBJGameHideKeyboard(unsigned long long surfaceId);
const char* TBJGameGetBundleRoot(unsigned long long surfaceId);
const char* TBJGameGetStorageRoot(unsigned long long surfaceId);
void TBJGameCopyToClipboard(const char* msg);
const char* TBJGameGetClipboardContent();
int TBJGameGetDeviceBulgeHeaderHeight();
void TBJGameStoreKeyValue(unsigned long long surfaceId, const char* key, const char* value);
void TBJGameRemoveStoreKey(unsigned long long surfaceId, const char* key);
const char* TBJGameGetStoreValue(unsigned long long surfaceId, const char* key);
void TBJGameClearStore(unsigned long long surfaceId);
const char* TBJGameGetCookie(unsigned long long surfaceId);

// Call in any thread
int TBJGameGetPathStatus(const char* path); // 0: not exist, 1: file, 2: dir
int TBJGameCreateDirectory(const char* dir);
int TBJGameDeletePath(const char* path);
int TBJGameCopyPath(const char* fromPath, const char* toPath);
void TBJGameGenerateUUID(char result[37]);
unsigned int TBJGamePlayAudio(const char* filePath, int loop, float pitch, float pan, float gain);
void TBJGameStopAudio(unsigned int audioId, int fadeOut);

//////// for debug /////////
void TBJGameBreakPoint(const char* msg);
//////////////////////////////////

//#ifdef TBJGAME_DESKTOP
unsigned long long TBJGameCreateSurface(const char* name, int width, int height);
typedef int (*TBJGameNativeMainCallBack)(int argc, char* argv[]);
int TBJGameNativeMain(TBJGameNativeMainCallBack callBack, int argc, char* argv[]);
void TBJGameNativeConfig(const char* content);
//#endif

TAOBAO_NS_END

#endif

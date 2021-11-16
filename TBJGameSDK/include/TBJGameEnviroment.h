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

typedef struct TBJGameKeyboardEvent
{
	int eventType;		// 0
	int keyboardType; // 0 : keyboard down. 1 : keyboard up. 2 : keyboard pressing.
	int keyWord;
} TBJGameKeyboardEvent;

#define TAOBAO_TOUCH_DOWN 0
#define TAOBAO_TOUCH_UP 1
#define TAOBAO_TOUCH_MOVE 2

typedef struct TBJGameTouchEvent
{
	int eventType;	// 1
	int touchType; // 0 : touch down. 1 : touch up. 2 : touch move.
    int touchId;
	float x;
	float y;
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

void TBJGameSetGameLoop(TBJGameLoop gameLoop);
void TBJGameSetEventListener(TBJGameEventListener eventListener);
// pass-through functions begin
void TBJGameExit();
int TBJGameCreateDirectory(const char* dir);
int TBJGameDeleteDirectory(const char* dir, int delEvenNotEmpty);
void TBJGameSwapBuffers(unsigned long long surfaceId);
int TBJGameGetFrameWidth(unsigned long long surfaceId);
int TBJGameGetFrameHeight(unsigned long long surfaceId);
void TBJGameSetFrameSize(unsigned long long surfaceId, int width, int height);
unsigned int TBJGamePlayAudio(const char* filePath, int loop, float pitch, float pan, float gain);
void TBJGameStopAudio(unsigned int audioId, int fadeOut);
// pass-through functions end

//////// for debug /////////
void TBJGameBreakPoint(const char* msg);
//////////////////////////////////

//#ifdef TBJGAME_DESKTOP
unsigned long long TBJGameCreateSurface(const char* name, int width, int height);
//#endif

TAOBAO_NS_END

#endif

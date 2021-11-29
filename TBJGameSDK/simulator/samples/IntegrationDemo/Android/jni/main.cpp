// FreetypeGraphicRenderer.cpp
// Copyright (C) 2011 Audiokinetic Inc
/// \file
/// Integration demo main.cpp implementation

#include <stdlib.h>
#include "AkFilePackageLowLevelIOBlocking.h"

// To see debug output of the Integraiton Demo:
// adb logcat ActivityManager:I IntegrationDemo:D *:S

// To see debug ouptut from the android_native_app_glue:
// adb logcat ActivityManager:I threaded_app *:S

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "IntegrationDemo", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "IntegrationDemo", __VA_ARGS__))

#include "IntegrationDemo.h"
#include "Drawing.h"
#include "InputMgr.h"
#include "FreetypeGraphicRenderer.h"

// Top 3/4th of the window
extern FreetypeGraphicRenderer* g_pFreetypeRenderer;

static bool g_isAppRunning = false;

static bool g_bAppIsInitialized = false;

// Alloc hook that need to be define by the game
namespace AK
{
	void * AllocHook( size_t in_size )
	{
		return malloc( in_size );
	}
	void FreeHook( void * in_ptr )
	{
		free( in_ptr );
	}
}


static void CopyRenderBufferToWindow( void* in_pBuffer, FreetypeGraphicRenderer * in_pRenderer, int in_iY = 0)
{
}

static void PostFrame( void* in_pBuffer )
{
	if(g_pFreetypeRenderer == NULL )
		return;

	CopyRenderBufferToWindow( in_pBuffer, g_pFreetypeRenderer );
}

static void TermApp(void* app)
{
	if(g_bAppIsInitialized)
	{
		g_bAppIsInitialized = false;
		IntegrationDemo::Instance().Term();	
	}
}

extern void DisplayKeyboard(void* app, bool pShow)
{
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
	IntegrationDemo::Instance().PauseAllSounds();
	IntegrationDemo::Instance().ResumeAllSounds();
	AkMemSettings memSettings;
	AkStreamMgrSettings stmSettings;
	AkDeviceSettings deviceSettings;
	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AkMusicSettings musicInit;
	IntegrationDemo::Instance().GetDefaultSettings(memSettings, stmSettings, deviceSettings, initSettings, platformInitSettings, musicInit);
	//IntegrationDemo::Instance().GetLowLevelIOHandler()->SetAssetManager(nullptr);
	IntegrationDemo::Instance().Init( memSettings, stmSettings, deviceSettings, initSettings, platformInitSettings, musicInit, NULL, nullptr, 256,11, 12);
	TermApp(app);
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
//void android_main(struct android_app* state)
int main(int args, char * * argv)
{
	IntegrationDemo::Instance().StartFrame();
	IntegrationDemo::Instance().Update();
	IntegrationDemo::Instance().Render();
	IntegrationDemo::Instance().EndFrame();
	return 0;
}

AkMemPoolId g_DefaultPoolId = 0;
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>

#define AK_ASYNC_OPEN_DEFAULT false

class CAkDefaultIOHookBlocking
{
public:
	// Initialization/termination. Init() registers this object as the one and 
	// only File Location Resolver if none were registered before. Then 
	// it creates a streaming device with scheduler type AK_SCHEDULER_BLOCKING.
	AKRESULT Init(
		const AkDeviceSettings &	in_deviceSettings,	// Device settings.
		bool						in_bAsyncOpen=AK_ASYNC_OPEN_DEFAULT	// If true, files are opened asynchronously when possible.
		);

	//-----------------------------------------------------------------------------------------
	// Sound Engine termination.
	//-----------------------------------------------------------------------------------------
	void Term();

	//-----------------------------------------------------------------------------------------
	// Access to LowLevelIO's file localization.
	//-----------------------------------------------------------------------------------------
	// File system interface.
	AKRESULT SetBasePath(
		const AkOSChar* in_pszBasePath
	);
};

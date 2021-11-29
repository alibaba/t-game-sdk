//////////////////////////////////////////////////////////////////////
//
// AkDefaultIOHookBlocking.cpp
//
// Default blocking low level IO hook (AK::StreamMgr::IAkIOHookBlocking) 
// and file system (AK::StreamMgr::IAkFileLocationResolver) implementation 
// on OS X. It can be used as a standalone implementation of the 
// Low-Level I/O system.
// 
// AK::StreamMgr::IAkFileLocationResolver: 
// Resolves file location using simple path concatenation logic 
// (implemented in ../Common/CAkFileLocationBase). It can be used as a 
// standalone Low-Level IO system, or as part of a multi device system. 
// In the latter case, you should manage multiple devices by implementing 
// AK::StreamMgr::IAkFileLocationResolver elsewhere (you may take a look 
// at class CAkDefaultLowLevelIODispatcher).
//
// AK::StreamMgr::IAkIOHookBlocking: 
// Uses the C Standard Input and Output Library.
// The AK::StreamMgr::IAkIOHookBlocking interface is meant to be used with
// AK_SCHEDULER_BLOCKING streaming devices. 
//
// Init() creates a streaming device (by calling AK::StreamMgr::CreateDevice()).
// AkDeviceSettings::uSchedulerTypeFlags is set inside to AK_SCHEDULER_BLOCKING.
// If there was no AK::StreamMgr::IAkFileLocationResolver previously registered 
// to the Stream Manager, this object registers itself as the File Location Resolver.
//
// Copyright (c) 2006 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AkDefaultIOHookBlocking.h"
//#include <android/asset_manager.h>


#define POSIX_BLOCKING_DEVICE_NAME		AKTEXT("Android Blocking")	// Default blocking device name.

CAkDefaultIOHookBlocking::CAkDefaultIOHookBlocking()
: m_deviceID( AK_INVALID_DEVICE_ID )
, m_bAsyncOpen( false )
{
}

CAkDefaultIOHookBlocking::~CAkDefaultIOHookBlocking()
{
}


// Initialization/termination. Init() registers this object as the one and 
// only File Location Resolver if none were registered before. Then 
// it creates a streaming device with scheduler type AK_SCHEDULER_BLOCKING.
AKRESULT CAkDefaultIOHookBlocking::Init(
	const AkDeviceSettings &	in_deviceSettings,		// Device settings.
	bool						in_bAsyncOpen/*=false*/	// If true, files are opened asynchronously when possible.
	)
{
	if ( in_deviceSettings.uSchedulerTypeFlags != AK_SCHEDULER_BLOCKING )
	{
		AKASSERT( !"CAkDefaultIOHookBlocking I/O hook only works with AK_SCHEDULER_BLOCKING devices" );
		return AK_Fail;
	}

	m_bAsyncOpen = in_bAsyncOpen;
	
	// If the Stream Manager's File Location Resolver was not set yet, set this object as the 
	// File Location Resolver (this I/O hook is also able to resolve file location).
	if ( !AK::StreamMgr::GetFileLocationResolver() )
		AK::StreamMgr::SetFileLocationResolver( this );

	CAkFileHelpers::Init();

	// Create a device in the Stream Manager, specifying this as the hook.
	m_deviceID = AK::StreamMgr::CreateDevice( in_deviceSettings, this );
	if ( m_deviceID != AK_INVALID_DEVICE_ID )
		return AK_Success;	

	return AK_Fail;
}

void CAkDefaultIOHookBlocking::Term()
{
	CAkFileHelpers::Term();
	if ( AK::StreamMgr::GetFileLocationResolver() == this )
		AK::StreamMgr::SetFileLocationResolver( NULL );
	AK::StreamMgr::DestroyDevice( m_deviceID );
}

//
// IAkFileLocationAware interface.
//-----------------------------------------------------------------------------

// Returns a file descriptor for a given file name (string).
AKRESULT CAkDefaultIOHookBlocking::Open( 
    const AkOSChar* in_pszFileName,     // File name.
    AkOpenMode      in_eOpenMode,       // Open mode.
    AkFileSystemFlags * in_pFlags,      // Special flags. Can pass NULL.
	bool &			io_bSyncOpen,		// If true, the file must be opened synchronously. Otherwise it is left at the File Location Resolver's discretion. Return false if Open needs to be deferred.
    AkFileDesc &    out_fileDesc        // Returned file descriptor.
    )
{
	// We normally consider that calls to ::CreateFile() on a hard drive are fast enough to execute in the
	// client thread. If you want files to be opened asynchronously when it is possible, this device should 
	// be initialized with the flag in_bAsyncOpen set to true.
	memset(&out_fileDesc, 0, sizeof(AkFileDesc));
	out_fileDesc.deviceID = m_deviceID;
	if ( io_bSyncOpen || !m_bAsyncOpen )
	{
		io_bSyncOpen = true;
		
		// Open the file without FILE_FLAG_OVERLAPPED and FILE_FLAG_NO_BUFFERING flags.
		return OpenFile(in_pszFileName,
						in_eOpenMode,
						in_pFlags,
						false,
						false,
						out_fileDesc );
	}

	// The client allows us to perform asynchronous opening.
	// We only need to specify the deviceID, and leave the boolean to false.
	out_fileDesc.deviceID = m_deviceID;
	return AK_Success;
}

// Returns a file descriptor for a given file ID.
AKRESULT CAkDefaultIOHookBlocking::Open( 
    AkFileID        in_fileID,          // File ID.
    AkOpenMode      in_eOpenMode,       // Open mode.
    AkFileSystemFlags * in_pFlags,      // Special flags. Can pass NULL.
	bool &			io_bSyncOpen,		// If true, the file must be opened synchronously. Otherwise it is left at the File Location Resolver's discretion. Return false if Open needs to be deferred.
    AkFileDesc &    out_fileDesc        // Returned file descriptor.
    )
{
	// We normally consider that calls to ::CreateFile() on a hard drive are fast enough to execute in the
	// client thread. If you want files to be opened asynchronously when it is possible, this device should 
	// be initialized with the flag in_bAsyncOpen set to true.
	memset(&out_fileDesc, 0, sizeof(AkFileDesc));
	out_fileDesc.deviceID = m_deviceID;
	if ( io_bSyncOpen || !m_bAsyncOpen )
	{
		io_bSyncOpen = true;
		
		// Open the file
		return CAkFileHelpers::OpenFile((AkFileID)in_fileID,
										in_eOpenMode,
										in_pFlags,
										false,
										false,
										out_fileDesc );
	}

	// The client allows us to perform asynchronous opening.
	// We only need to specify the deviceID, and leave the boolean to false.
	out_fileDesc.deviceID = m_deviceID;
	return AK_Success;
}

//
// IAkIOHookBlocking implementation.
//-----------------------------------------------------------------------------
// Reads data from a file (synchronous). 
AKRESULT CAkDefaultIOHookBlocking::Read(
    AkFileDesc &			in_fileDesc,        // File descriptor.
	const AkIoHeuristics & /*in_heuristics*/,	// Heuristics for this data transfer (not used in this implementation).
    void *					out_pBuffer,        // Buffer to be filled with data.
    AkIOTransferInfo &		io_transferInfo		// Synchronous data transfer info. 
    )
{
	AkUInt32 out_readSize = 0;
	return CAkFileHelpers::ReadBlocking(
		in_fileDesc,
		out_pBuffer, 
		(AkUInt32)io_transferInfo.uFilePosition, 
		io_transferInfo.uRequestedSize,
		out_readSize);
}

// Writes data to a file (synchronous). 
AKRESULT CAkDefaultIOHookBlocking::Write(
	AkFileDesc &			in_fileDesc,        // File descriptor.
	const AkIoHeuristics & /*in_heuristics*/,	// Heuristics for this data transfer (not used in this implementation).
    void *					in_pData,           // Data to be written.
    AkIOTransferInfo &		io_transferInfo		// Synchronous data transfer info. 
    )
{
	return CAkFileHelpers::Write(in_fileDesc, in_pData, io_transferInfo);
}

// Cleans up a file.
AKRESULT CAkDefaultIOHookBlocking::Close( AkFileDesc & in_fileDesc )
{
	CAkFileHelpers::CloseFile( in_fileDesc );
	return AK_Success;
}

// Returns the block size for the file or its storage device. 
AkUInt32 CAkDefaultIOHookBlocking::GetBlockSize( AkFileDesc &)
{
	// No constraint on block size (file seeking).
    return 1;
}


// Returns a description for the streaming device above this low-level hook.
void CAkDefaultIOHookBlocking::GetDeviceDesc(
    AkDeviceDesc &  
#ifndef AK_OPTIMIZED
	out_deviceDesc      // Description of associated low-level I/O device.
#endif
    )
{
#ifndef AK_OPTIMIZED
	out_deviceDesc.deviceID       = m_deviceID;
	out_deviceDesc.bCanRead       = true;
	out_deviceDesc.bCanWrite      = true;
	
	AK_OSCHAR_TO_UTF16( out_deviceDesc.szDeviceName, POSIX_BLOCKING_DEVICE_NAME, AK_MONITOR_DEVICENAME_MAXLENGTH );
	out_deviceDesc.uStringSize   = (AkUInt32)AKPLATFORM::AkUtf16StrLen( out_deviceDesc.szDeviceName ) + 1;
#endif
}

// Returns custom profiling data: 1 if file opens are asynchronous, 0 otherwise.
AkUInt32 CAkDefaultIOHookBlocking::GetDeviceData()
{
	return ( m_bAsyncOpen ) ? 1 : 0;
}

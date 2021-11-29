//////////////////////////////////////////////////////////////////////
//
// AkFileHelpers.h
//
// Platform-specific helpers for files.
//
// Copyright (c) 2006 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

#ifndef _AK_FILE_HELPERS_H_
#define _AK_FILE_HELPERS_H_

#include <AK/Tools/Common/AkAssert.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
//#include <android/asset_manager.h>
#include "../Common/AkFileLocationBase.h"
#include <AK/Tools/Common/AkListBareLight.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>

#include <sys/stat.h> 

struct AAssetManager;

class CAkFileLocation : public CAkFileLocationBase
{
public:
	CAkFileLocation() : pNextLightItem(NULL)
	{}

	virtual AKRESULT OpenFile( 
		const AkOSChar* in_pszFilename,     // File name.
		AkOpenMode      in_eOpenMode,       // Open mode.		
		bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
		bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
		AkFileDesc &	out_descFile        // Returned file identifier/handle.
		) = 0;

	// Wrapper for system file handle closing.
	virtual AKRESULT CloseFile( AkFileHandle in_hFile ) = 0;

	virtual AKRESULT Write(
		AkFileHandle &	in_hFile,			        // File descriptor.
		void *					in_pData,           // Data to be written.
		AkIOTransferInfo &		io_transferInfo		// Synchronous data transfer info. 
		){return AK_Fail;}

	//
	// Simple platform-independent API to open and read files using AkFileHandles, 
	// with blocking calls and minimal constraints.
	// ---------------------------------------------------------------------------

	// Open file to use with ReadBlocking().
	AKRESULT OpenBlocking(
		const AkOSChar* in_pszFilename,     // File name.
		AkFileDesc &  out_descFile           // Returned file handle.
		)
	{
		return OpenFile( 
			in_pszFilename,
			AK_OpenModeRead,
			false,
			false, 
			out_descFile );
	}

	// Simple blocking read method.
	virtual AKRESULT ReadBlocking(
		AkFileHandle &	in_hFile,			// Returned file identifier/handle.
		void *			in_pBuffer,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.
		AkUInt32		in_uPosition,		// Position from which to start reading.
		AkUInt32		in_uSizeToRead,		// Size to read. Must be a multiple of CAkFileHelpers::s_uRequiredBlockSize.
		AkUInt32 &		out_uSizeRead		// Returned size read.        
		) = 0;

	virtual AKRESULT CheckDirectoryExists( const AkOSChar* in_pszBasePath ) = 0;

	CAkFileLocation *pNextLightItem;
};

class CAkAPKLocation : public CAkFileLocation
{
public:

	virtual AKRESULT OpenFile( 
        const AkOSChar* in_pszFilename,     // File name.
        AkOpenMode      in_eOpenMode,       // Open mode.
        bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
        bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
        AkFileDesc &  out_descFile           // Returned file identifier/handle.
        );

	// Wrapper for system file handle closing.
	virtual AKRESULT CloseFile( AkFileHandle in_hFile );

	// Simple blocking read method.
	virtual AKRESULT ReadBlocking(
        AkFileHandle &	in_hFile,			// Returned file identifier/handle.
		void *			in_pBuffer,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.
		AkUInt32		in_uPosition,		// Position from which to start reading.
		AkUInt32		in_uSizeToRead,		// Size to read. Must be a multiple of CAkFileHelpers::s_uRequiredBlockSize.
		AkUInt32 &		out_uSizeRead		// Returned size read.        
		);

	virtual AKRESULT CheckDirectoryExists( const AkOSChar* in_pszBasePath );

	void SetAssetManager(AAssetManager* assetManager) { ms_assetManager = assetManager; }

private:
	AAssetManager*  ms_assetManager;
};

class CAkPOSIXLocation : public CAkFileLocation
{
public:

	virtual AKRESULT OpenFile( 
		const AkOSChar* in_pszFilename,     // File name.
		AkOpenMode      in_eOpenMode,       // Open mode.
		bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
		bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
		AkFileDesc &  out_descFile           // Returned file identifier/handle.
		);	

	// Wrapper for system file handle closing.
	virtual AKRESULT CloseFile( AkFileHandle in_hFile );

	// Simple blocking read method.
	virtual AKRESULT ReadBlocking(
		AkFileHandle &	in_hFile,			// Returned file identifier/handle.
		void *			in_pBuffer,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.
		AkUInt32		in_uPosition,		// Position from which to start reading.
		AkUInt32		in_uSizeToRead,		// Size to read. Must be a multiple of CAkFileHelpers::s_uRequiredBlockSize.
		AkUInt32 &		out_uSizeRead		// Returned size read.        
		);

	virtual AKRESULT Write(
		AkFileHandle &	in_hFile,			        // File descriptor.
		void *					in_pData,           // Data to be written.
		AkIOTransferInfo &		io_transferInfo		// Synchronous data transfer info. 
		);

		virtual AKRESULT CheckDirectoryExists( const AkOSChar* in_pszBasePath );
};

class CAkFileHelpers
{
public:

	CAkFileHelpers();

	void Init();
	void Term();

	AKRESULT OpenFile( 
		const AkOSChar* in_pszFilename,     // File name.
		AkOpenMode      in_eOpenMode,       // Open mode.
		AkFileSystemFlags * in_pFlags,
		bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
		bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
		AkFileDesc &    out_fileDesc          // Returned file identifier/handle.
		);

	AKRESULT OpenFile( 
		AkFileID        in_fileID,			// File id.
		AkOpenMode      in_eOpenMode,       // Open mode.
		AkFileSystemFlags * in_pFlags,
		bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
		bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
		AkFileDesc &    out_fileDesc          // Returned file identifier/handle.
		);

	inline AKRESULT Write(
		AkFileDesc &			in_fileDesc,        // File descriptor.
		void *					in_pData,           // Data to be written.
		AkIOTransferInfo &		io_transferInfo		// Synchronous data transfer info. 
		)
	{
		CAkFileLocation* pLoc = (CAkFileLocation*)in_fileDesc.pCustomParam;
		return pLoc->Write(in_fileDesc.hFile, in_pData, io_transferInfo);
	}

	// Open file to use with ReadBlocking().
	inline AKRESULT OpenBlocking(
		const AkOSChar* in_pszFilename,     // File name.
		AkFileDesc &    out_fileDesc           // Returned file handle.
		)
	{
		return OpenFile( 
			in_pszFilename,
			AK_OpenModeRead,
			NULL,
			false,
			false, 
			out_fileDesc );
	}

	// Wrapper for system file handle closing.
	inline void CloseFile( AkFileDesc & out_fileDesc )
	{
		CAkFileLocation* pLoc = (CAkFileLocation*)out_fileDesc.pCustomParam;
		pLoc->CloseFile(out_fileDesc.hFile);
	}

	// Simple blocking read method.
	inline AKRESULT ReadBlocking(
		AkFileDesc & in_fileDesc,			// Returned file identifier/handle.
		void *			in_pBuffer,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.
		AkUInt32		in_uPosition,		// Position from which to start reading.
		AkUInt32		in_uSizeToRead,		// Size to read. Must be a multiple of CAkFileHelpers::s_uRequiredBlockSize.
		AkUInt32 &		out_uSizeRead		// Returned size read.        
		)
	{
		CAkFileLocation* pLoc = (CAkFileLocation*)in_fileDesc.pCustomParam;
		return pLoc->ReadBlocking(in_fileDesc.hFile, in_pBuffer, in_uPosition, in_uSizeToRead, out_uSizeRead);
	}

	// Set Android asset manager to file helper.
	void SetAssetManager(AAssetManager* assetManager);	

	/// Sets the base path for file loading.  This path must be inside the app's APK
	AKRESULT SetBasePath(const AkOSChar* in_pszPath);

	/// Adds a alternative path for file loading.  This path can be anywhere in the file system.  Multiple paths can be specified.  Last path specified is first to be searched for files.
	AKRESULT AddBasePath(const AkOSChar* in_pszPath);
	
	/// Returns AK_Success if the directory is valid, AK_Fail if not.
	/// For validation purposes only.
	/// Some platforms may return AK_NotImplemented, in this case you cannot rely on it.
	static AKRESULT CheckDirectoryExists( const AkOSChar* in_pszBasePath )
	{
		return AK_Success;
	}

protected:
	AkListBareLight<CAkFileLocation> m_Locations;
	CAkAPKLocation m_APKLocation;	//There is always an APK.
};


#endif //_AK_FILE_HELPERS_H_

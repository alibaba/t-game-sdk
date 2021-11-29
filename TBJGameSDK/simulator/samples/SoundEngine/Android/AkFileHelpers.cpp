//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

#include "AkFileHelpers.h"
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/Tools/Common/AkObject.h>
#include <sys/stat.h>

CAkFileHelpers::CAkFileHelpers()
{	
}

void CAkFileHelpers::Init()
{
	m_Locations.AddFirst(&m_APKLocation);
}


void CAkFileHelpers::Term()
{
	for(AkListBareLight<CAkFileLocation>::Iterator it = m_Locations.Begin(); it != m_Locations.End();)
	{
		CAkFileLocation *p = (*it);
		++it;
		if (p != &m_APKLocation)
			AkDelete(AK::StreamMgr::GetPoolID(), p);
	}
	m_Locations.Term();
}

AKRESULT CAkFileHelpers::OpenFile( 
				  const AkOSChar* in_pszFilename,     // File name.
				  AkOpenMode      in_eOpenMode,       // Open mode.
				  AkFileSystemFlags * in_pFlags,      // Special flags. Can pass NULL.
				  bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
				  bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
				  AkFileDesc &    out_fileDesc           // Returned file identifier/handle.
				  )
{		
	AKRESULT res = AK_FileNotFound;
	//Go through all locations in order.
	for(AkListBareLight<CAkFileLocation>::Iterator it = m_Locations.Begin(); it != m_Locations.End() && res != AK_Success; ++it)
	{
		AkOSChar szFullFilePath[AK_MAX_PATH];
		if ( (*it)->GetFullFilePath( in_pszFilename, in_pFlags, in_eOpenMode, szFullFilePath ) == AK_Success )
		{
			res = (*it)->OpenFile(szFullFilePath, in_eOpenMode, in_bOverlappedIO, in_bUnbufferedIO, out_fileDesc);
			out_fileDesc.pCustomParam = (*it);			
		}
	}

	return res;
}

AKRESULT CAkFileHelpers::OpenFile( 
								  AkFileID        in_fileID,	
								  AkOpenMode      in_eOpenMode,       // Open mode.
								  AkFileSystemFlags * in_pFlags,      // Special flags. Can pass NULL.
								  bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
								  bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
								  AkFileDesc &    out_fileDesc           // Returned file identifier/handle.
								  )
{
	AKRESULT res = AK_FileNotFound;
	//Go through all locations in order.
	for(AkListBareLight<CAkFileLocation>::Iterator it = m_Locations.Begin(); it != m_Locations.End() && res != AK_Success; ++it)
	{
		AkOSChar szFullFilePath[AK_MAX_PATH];
		if ( (*it)->GetFullFilePath( in_fileID, in_pFlags, in_eOpenMode, szFullFilePath ) == AK_Success )
		{
			res = (*it)->OpenFile(szFullFilePath, in_eOpenMode, in_bOverlappedIO, in_bUnbufferedIO, out_fileDesc);
			out_fileDesc.pCustomParam = (*it);			
		}
	}

	return res;
}


void CAkFileHelpers::SetAssetManager(AAssetManager* assetManager)
{
	m_APKLocation.SetAssetManager(assetManager);
}

AKRESULT CAkFileHelpers::SetBasePath(const AkOSChar* in_pszPath)
{
	AKRESULT res = m_APKLocation.SetBasePath(in_pszPath);
	if (res != AK_Success)
		return res;

	//Yes, this is checked AFTER the base path has been set on the location object.  It is legal that the directory doesn't exist at startup.  
	//However, an error code should be returned for information purposes so the client knows why the files are not found.
	return m_APKLocation.CheckDirectoryExists(in_pszPath);	
}

AKRESULT CAkFileHelpers::AddBasePath(const AkOSChar* in_pszPath)
{	
	if (AK::StreamMgr::GetPoolID() == AK_INVALID_POOL_ID)
		return AK_StreamMgrNotInitialized;

	CAkPOSIXLocation * pLoc = AkNew(AK::StreamMgr::GetPoolID(), CAkPOSIXLocation());
	if (pLoc == NULL)
		return AK_InsufficientMemory;
	
	pLoc->SetBasePath(in_pszPath);	
	m_Locations.AddFirst(pLoc); //Last added is the first searched.

	//Yes, this is checked AFTER the base path has been set on the location object.  It is legal that the directory doesn't exist at startup.  
	//However, an error code should be returned for information purposes so the client knows why the files are not found.
	return pLoc->CheckDirectoryExists(in_pszPath);	
}

//--------------------------------------------------------------------

AKRESULT CAkAPKLocation::OpenFile( 
						  const AkOSChar* in_pszFilename,     // File name.
						  AkOpenMode      in_eOpenMode,       // Open mode.
						  bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
						  bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
						  AkFileDesc &  out_descFile           // Returned file identifier/handle.
						  )
{
	// Check parameters.
	if ( ! in_pszFilename )
	{
		AKASSERT( !"NULL file name" );
		return AK_InvalidParameter;
	}

	if ( ! ms_assetManager )
	{
		AKASSERT( !"Android Asset Manager is not set" );
		return AK_Fail;
	}	

	AAsset* asset = AAssetManager_open(ms_assetManager, in_pszFilename, AASSET_MODE_UNKNOWN);
	if ( ! asset ) 
		return AK_FileNotFound;

	out_descFile.hFile = (AkFileHandle) asset;
	AkInt64 fileSize = AAsset_getLength((AAsset*)out_descFile.hFile);
	out_descFile.iFileSize			= fileSize;
	out_descFile.uSector			= 0;

	return AK_Success;
}

// Wrapper for system file handle closing.
AKRESULT CAkAPKLocation::CloseFile( AkFileHandle in_hFile )
{
	AAsset_close( (AAsset*) in_hFile);
	return AK_Success;
}	

// Simple blocking read method.
AKRESULT CAkAPKLocation::ReadBlocking(
							  AkFileHandle &	in_hFile,			// Returned file identifier/handle.
							  void *			in_pBuffer,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.
							  AkUInt32		in_uPosition,		// Position from which to start reading.
							  AkUInt32		in_uSizeToRead,		// Size to read. Must be a multiple of CAkFileHelpers::s_uRequiredBlockSize.
							  AkUInt32 &		out_uSizeRead		// Returned size read.        
							  )
{
	AKASSERT( in_uSizeToRead % s_uRequiredBlockSize == 0 && in_uPosition % s_uRequiredBlockSize == 0 );

	AAsset* asset = (AAsset*) in_hFile;
	// fpos_t pos = io_transferInfo.uFilePosition;
	if( AAsset_seek( asset, in_uPosition, SEEK_SET ) == -1 )
	{
		return AK_Fail;
	}

	out_uSizeRead = (AkUInt32) AAsset_read(asset, in_pBuffer, in_uSizeToRead);
	if( out_uSizeRead == in_uSizeToRead)
		return AK_Success;

	return AK_Fail;		
}

/// Returns AK_Success if the directory is valid, AK_Fail if not.
/// For validation purposes only.
AKRESULT CAkAPKLocation::CheckDirectoryExists( const AkOSChar* in_pszBasePath )
{
	AAssetDir* pDir = AAssetManager_openDir(ms_assetManager, in_pszBasePath);	
	if (pDir)
	{
		AAssetDir_close(pDir);
		return AK_Success;
	}
	return AK_PathNotFound;
}


//--------------------------------------------------------------------

AKRESULT CAkPOSIXLocation::OpenFile( 
						  const AkOSChar* in_pszFilename,     // File name.
						  AkOpenMode      in_eOpenMode,       // Open mode.
						  bool            in_bOverlappedIO,	// Use FILE_FLAG_OVERLAPPED flag.
						  bool            in_bUnbufferedIO,   // Use FILE_FLAG_NO_BUFFERING flag.
						  AkFileDesc &  out_descFile           // Returned file identifier/handle.
						  )
{
	// Check parameters.
	if ( !in_pszFilename )
	{
		AKASSERT( !"NULL file name" );
		return AK_InvalidParameter;
	}

	char* mode;
	switch ( in_eOpenMode )
	{
	case AK_OpenModeRead:
		mode = (char*)"r"; 
		break;
	case AK_OpenModeWrite:
		mode = (char*)"w";
		break;
	case AK_OpenModeWriteOvrwr:
		mode = (char*)"w+";
		break;
	case AK_OpenModeReadWrite:
		mode = (char*)"a";
		break;
	default:
		AKASSERT( !"Invalid open mode" );
		out_descFile.hFile = NULL;
		return AK_InvalidParameter;
	}

	out_descFile.hFile = fopen( in_pszFilename , mode );

	if( !out_descFile.hFile )
		return AK_FileNotFound;

	struct stat buf;
	// Get Info about the file size
	if( stat(in_pszFilename, &buf) != 0)
		return AK_Fail;

	out_descFile.iFileSize = buf.st_size;

	return AK_Success;
}

// Wrapper for system file handle closing.
AKRESULT CAkPOSIXLocation::CloseFile( AkFileHandle in_hFile )
{
	if ( !fclose( in_hFile ) )
		return AK_Success;

	AKASSERT( !"Failed to close file handle" );
	return AK_Fail;
}

// Simple blocking read method.
AKRESULT CAkPOSIXLocation::ReadBlocking(
							  AkFileHandle &	in_hFile,			// Returned file identifier/handle.
							  void *			in_pBuffer,			// Buffer. Must be aligned on CAkFileHelpers::s_uRequiredBlockSize boundary.
							  AkUInt32		in_uPosition,		// Position from which to start reading.
							  AkUInt32		in_uSizeToRead,		// Size to read. Must be a multiple of CAkFileHelpers::s_uRequiredBlockSize.
							  AkUInt32 &		out_uSizeRead		// Returned size read.        
							  )
{
	AKASSERT( in_uSizeToRead % s_uRequiredBlockSize == 0 && in_uPosition % s_uRequiredBlockSize == 0 );
	if( fseek(in_hFile, in_uPosition, SEEK_SET ) )
	{
		return AK_Fail;
	}

	out_uSizeRead = fread( in_pBuffer, 1, in_uSizeToRead , in_hFile );
	if( out_uSizeRead == in_uSizeToRead )
	{
		return AK_Success;
	}
	return AK_Fail;		
}

AKRESULT CAkPOSIXLocation::Write(
	AkFileHandle &	in_hFile,			        // File descriptor.
	void *					in_pData,           // Data to be written.
	AkIOTransferInfo &		io_transferInfo		// Synchronous data transfer info. 
	)
{
	size_t written = fwrite(in_pData, 1, io_transferInfo.uRequestedSize, in_hFile);
	return written == io_transferInfo.uRequestedSize ? AK_Success : AK_Fail;
}

/// Returns AK_Success if the directory is valid, AK_Fail if not.
/// For validation purposes only.
AKRESULT CAkPOSIXLocation::CheckDirectoryExists( const AkOSChar* in_pszBasePath )
{
	struct stat status;
	stat( in_pszBasePath, &status );
	if( status.st_mode & S_IFDIR )
		return AK_Success;

	return AK_PathNotFound;
}

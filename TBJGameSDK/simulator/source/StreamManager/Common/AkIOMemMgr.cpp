//////////////////////////////////////////////////////////////////////
//
// AkIOMemMgr.cpp
//
// IO memory management.
//
// Copyright (c) 2006 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AkIOMemMgr.h"
#include <stdio.h>

using namespace AK;
using namespace AK::StreamMgr;

CAkIOMemMgr::CAkIOMemMgr()
: m_pIOMemory( NULL )
, m_pMemBlocks( NULL )
, m_streamIOPoolId( AK_INVALID_POOL_ID )
, m_uNumViewsAvail( 0 )
, m_bUseCache( false )
#ifndef AK_OPTIMIZED
, m_streamIOPoolSize( 0 )
, m_uAllocs( 0 )
, m_uFrees( 0 )
, m_uPeakUsed( 0 )
#endif
{
}

CAkIOMemMgr::~CAkIOMemMgr()
{
}

AKRESULT CAkIOMemMgr::Init( 
	const AkDeviceSettings &	in_settings 
	)
{
	// Number of I/O buffers and effective pool size:
	AkUInt32 uNumBuffers = in_settings.uIOMemorySize / in_settings.uGranularity;
	AkUInt32 uMemorySize = uNumBuffers * in_settings.uGranularity;

	// Create stream memory pool.
    if ( uMemorySize > 0 )
    {		
		m_streamIOPoolId = AK::MemoryMgr::CreatePool( 
            in_settings.pIOMemory,
			uMemorySize,
			uMemorySize,
			in_settings.ePoolAttributes | AkFixedSizeBlocksMode,
			in_settings.uIOMemoryAlignment );        
    }

    if( m_streamIOPoolId != AK_INVALID_POOL_ID )
	{
        // This pool must not send error notifications: not having memory is normal and notifications are costly.
        AK::MemoryMgr::SetMonitoring(
            m_streamIOPoolId,
            false );

        AK_SETPOOLNAME( m_streamIOPoolId, AKTEXT("Stream I/O") );

#ifndef AK_OPTIMIZED
		m_streamIOPoolSize = uMemorySize;
#endif

		// "Allocate" all pool and create I/O blocks pool.
		m_pIOMemory = AK::MemoryMgr::GetBlock( m_streamIOPoolId );
		AKASSERT( m_pIOMemory );	// Has to succeed.

		m_pMemBlocks = (AkMemBlock*)AkAlloc( CAkStreamMgr::GetObjPoolID(), uNumBuffers * sizeof( AkMemBlock ) );
		if ( !m_pMemBlocks )
		{
			AKASSERT( !"Not enough memory in the stream manager pool to index stream IO buffers" );
			return AK_Fail;
		}

		AkUInt8 * pIOMemory = (AkUInt8*)m_pIOMemory;
		AkMemBlock * pBlock = m_pMemBlocks;
		const AkMemBlock * pBlockEnd = pBlock + uNumBuffers;
		do
		{
			AkPlacementNew( pBlock ) AkMemBlock( pIOMemory );
			pIOMemory += in_settings.uGranularity;
			m_listFreeBuffers.AddLast( pBlock++ );
		}
		while ( pBlock < pBlockEnd );
		
		// Create cached memory dictionnary.
		if ( m_arMemBlocks.Reserve( uNumBuffers ) != AK_Success )
		{
			AKASSERT( !"Not enough memory in the stream manager pool to create cache repository" );
			return AK_Fail;
		}
		for ( AkUInt16 uIndex = 0; uIndex<uNumBuffers; uIndex++ )
		{
			m_arMemBlocks.AddLast( uIndex );
		}

		// Compute number of views available in order to respect the maximum cache ratio.
		m_uNumViewsAvail = (AkUInt32)( in_settings.fMaxCacheRatio * uNumBuffers + 0.5f );
		if ( m_uNumViewsAvail < uNumBuffers )
			m_uNumViewsAvail = uNumBuffers;
		m_bUseCache = ( in_settings.fMaxCacheRatio > 1.f );
	}
    else if ( in_settings.uIOMemorySize > 0 )
    {
        AKASSERT( !"Cannot create stream pool, or IO memory size is smaller than granularity" );
		return AK_Fail;
    }
    // otherwise, device does not support automatic streams.

	return AK_Success;
}

void CAkIOMemMgr::Term()
{
	// Free IO memory index.
#ifdef _DEBUG
	// Ensure all blocks have been freed properly.
	AkMemBlocksDictionnary::Iterator it = m_arMemBlocks.Begin();
	while ( it != m_arMemBlocks.End() )
	{
		AKASSERT( (IndexToMemBlock(*it))->uRefCount == 0 );
		++it;
	}
#endif
	m_arMemBlocks.Term();

	if ( !m_listFreeBuffers.IsEmpty() )
	{
		m_listFreeBuffers.RemoveAll();
		AkFree( CAkStreamMgr::GetObjPoolID(), m_pMemBlocks );
	}
	m_listFreeBuffers.Term();

    // Destroy IO pool.
	if ( m_streamIOPoolId != AK_INVALID_POOL_ID )
	{
		AK::MemoryMgr::ReleaseBlock( m_streamIOPoolId, m_pIOMemory );
        AKVERIFY( AK::MemoryMgr::DestroyPool( m_streamIOPoolId ) == AK_Success );
		m_streamIOPoolId = AK_INVALID_POOL_ID;
	}
}

// IO memory access.
// IMPORTANT: These methods are not thread safe. 
//

// Release memory block.
// Returns refcount after releasing.
AkUInt32 CAkIOMemMgr::ReleaseBlock(
	AkMemBlock *	in_pMemBlock	// Memory block to release.
	)
{
	AKASSERT( in_pMemBlock->uRefCount > 0 );
	AkUInt32 uRefCount = --in_pMemBlock->uRefCount;
	if ( 0 == in_pMemBlock->uRefCount )
	{
		AKASSERT( !in_pMemBlock->pTransfer || !"Freeing block that has transfer" );

		// Add on top of the FIFO if the block is not tagged, at the end otherwise.
		if ( in_pMemBlock->IsTagged() )
			m_listFreeBuffers.AddLast( in_pMemBlock );
		else
			m_listFreeBuffers.AddFirst( in_pMemBlock );

#ifndef AK_OPTIMIZED
		++m_uFrees;
#endif
	}

	++m_uNumViewsAvail;
	AKASSERT( m_uNumViewsAvail >= m_listFreeBuffers.Length() );

	return uRefCount;
}

// Get a free memory block.
// Returned memory block is addref'd (to 1), NULL if none found.
void CAkIOMemMgr::GetOldestFreeBlock(
	AkMemBlock *&	out_pMemBlock
	)
{
	out_pMemBlock = m_listFreeBuffers.First();
	if ( out_pMemBlock )
	{
		AKASSERT( out_pMemBlock->uRefCount == 0 );
		++out_pMemBlock->uRefCount;

		m_listFreeBuffers.RemoveFirst();

		--m_uNumViewsAvail;
		AKASSERT( m_uNumViewsAvail >= m_listFreeBuffers.Length() );

#ifndef AK_OPTIMIZED
		++m_uAllocs;
#endif

		// pNextBlock is not used now that the block was dequeued, but it is an union with pTransfer, 
		// so we ought to clear it now.
		out_pMemBlock->pTransfer = NULL;
	}
}

#ifdef _DEBUG
// Debugging: Verify that cache is consistent.
void CAkIOMemMgr::CheckCacheConsistency( AkMemBlock * in_pMustFindBlock )
{
	AkMemBlock * pPrevBlock = NULL;
	bool bFound = ( in_pMustFindBlock == NULL );
	AkMemBlocksDictionnary::Iterator it = m_arMemBlocks.Begin();
	while ( it != m_arMemBlocks.End() )
	{
		AkMemBlock * pThisBlock = IndexToMemBlock((*it));
		if ( pPrevBlock 
			&& ( pPrevBlock->fileID > pThisBlock->fileID
				|| ( pPrevBlock->fileID == pThisBlock->fileID && pPrevBlock->uPosition < pThisBlock->uPosition )
				|| ( pPrevBlock->fileID == pThisBlock->fileID && pPrevBlock->uPosition == pThisBlock->uPosition && pPrevBlock->pData > pThisBlock->pData )
				) )
		{
			AKASSERT( false );
		}
		if ( in_pMustFindBlock == pThisBlock )
			bFound = true;

		pPrevBlock = pThisBlock;
		++it;
	}
	AKASSERT( bFound );
}
#endif

#ifdef AK_STREAM_MGR_INSTRUMENTATION
void CAkIOMemMgr::PrintCache()
{
	AKPLATFORM::OutputDebugMsg( "Index FileID Position Size\n" );
	AkMemBlocksDictionnary::Iterator it = m_arMemBlocks.Begin();
	while ( it != m_arMemBlocks.End() )
	{
		AkMemBlock * pMemBlock = IndexToMemBlock((*it));
		char msg[64];
		sprintf( msg, "%u\t%u\t%lu\t%u\n", (*it), pMemBlock->fileID, (AkUInt32)pMemBlock->uPosition, pMemBlock->uAvailableSize );
		AKPLATFORM::OutputDebugMsg( msg );
		++it;
	}
}
#endif

// Comparison function for cache info binary search, using the first 2 keys: file ID and end position.
// Favors going towards the entry having the largest end position: Use this to converge to the first 
// element that matches the two criteria.
// Returns -1 if a better match _could_ be found before, 1 if a better match would be found after,
// 0 if a perfect match is found.
inline int Compare_FileID_Position( const AkMemBlock * in_pSorted, AkFileID in_fileID, AkUInt64 in_uPosition )
{
	// 1st key: file ID.
	if ( in_fileID < in_pSorted->fileID )
		return -1;
	else if ( in_fileID > in_pSorted->fileID )
		return 1;

	// 2nd key: position.
	else if ( in_uPosition > in_pSorted->uPosition )
		return -1;
	else if ( in_uPosition < in_pSorted->uPosition )
		return 1;

	AKASSERT( in_uPosition == in_pSorted->uPosition && in_fileID == in_pSorted->fileID );
	return 0;
}

// Finds a cached memory block if available.
// If a match was found, out_pMemBlock is set and addref'd. NULL otherwise.
// Returns the offset that is useable for the client within the cache buffer. 
// The actual data size io_uRequestedSize is updated, and is always between in_uMinSize and 
// io_uRequestedSize, except if it is the last memory region of the file.
// Returns the useable address within the cache buffer if a match was found, NULL otherwise.
// Notes: 
// - Returned memory block is addref'd.
// - Alignment and minimum data size constraint applicable to cache are handled herein.
AkUInt32 CAkIOMemMgr::GetCachedBlock(
	AkFileID		in_fileID,			// Block's associated file ID.
	AkUInt64		in_uPosition,		// Desired position in file.
	AkUInt32		in_uMinSize,		// Minimum data size acceptable (discard otherwise).
	AkUInt32		in_uRequiredAlign,	// Required data alignment.
	bool			in_bEof,			// True if last buffer in file
	AkUInt32 &		io_uRequestedSize,	// In: desired data size; Out: returned valid size. Modified only if a block is found.
	AkMemBlock *&	out_pMemBlock		// Returned memory block.
	)
{
	AKASSERT( UseCache() );
	AKASSERT( in_fileID != AK_INVALID_FILE_ID 
			&& in_uRequiredAlign >= 1 
			&& io_uRequestedSize >= in_uMinSize );

	out_pMemBlock = NULL;

	// Check if max caching was reached.
	AKASSERT( m_uNumViewsAvail >= m_listFreeBuffers.Length() );
	if ( m_uNumViewsAvail == m_listFreeBuffers.Length() )
	{
		// Cannot reference cached data anymore. Small object pool is going to blow. Bail out.
		return 0;
	}

	AkMemBlock * pMemBlock = NULL;

	PRINT_CACHE();
	
	// 1) Perform a binary search to bring us just before or right on the block that matches the file ID and 
	// data position.
	int iTop = 0, iBottom = m_arMemBlocks.Length()-1;
	int iThis = ( iBottom - iTop ) / 2 + iTop; 
	do
	{
		iThis = ( iBottom - iTop ) / 2 + iTop; 
		int iCmp = Compare_FileID_Position( IndexToMemBlock(m_arMemBlocks[iThis]), in_fileID, in_uPosition );
		if ( 0 == iCmp )
		{
			pMemBlock = IndexToMemBlock(m_arMemBlocks[iThis]);
			break;
		}
		else if ( iCmp < 0 )
			iBottom = iThis - 1;
		else
			iTop = iThis + 1;
	}
	while ( iTop <= iBottom );

	// if (pMemBlock), iThis now points the block with same file ID and position.
	// Otherwise, iTop OR iBottom (depending on the direction of last bisection) points to the item just 
	// before (which may or may not have the correct file ID, and if it does, has the next larger position). 
	// This item may be useable. Whether a perfect match was found or not, we still need to ensure that it is.
	if ( !pMemBlock 
		|| ( in_uPosition > pMemBlock->uPosition + io_uRequestedSize - in_uMinSize ) )	// Perfect match, but is it valid?
	{
		// No perfect match, or perfect match not valid.
		// The current pick may be usable.
		AkMemBlock * pCurrentPick = IndexToMemBlock(m_arMemBlocks[iThis]);
		if ( in_fileID == pCurrentPick->fileID
			&& in_uPosition >= pCurrentPick->uPosition
			&& in_uPosition <= pCurrentPick->uPosition + pCurrentPick->uAvailableSize - in_uMinSize )
		{
			pMemBlock = pCurrentPick;
		}
		else
		{
			// Otherwise the next block may also be usable.
			int iNextBlock = iThis + 1;
			if( iNextBlock < (AkInt32)m_arMemBlocks.Length() )
			{
				AkMemBlock * pSecondBestBlock = IndexToMemBlock(m_arMemBlocks[iNextBlock]);
				if ( in_fileID == pSecondBestBlock->fileID
					&& in_uPosition >= pSecondBestBlock->uPosition
					&& in_uPosition <= pSecondBestBlock->uPosition + pSecondBestBlock->uAvailableSize - in_uMinSize )
				{
					pMemBlock = pSecondBestBlock;
				}
				else
				{
					// No.
					return 0;
				}
			}
			else
			{
				// No.
				return 0;
			}
		}
	}

	AKASSERT( pMemBlock );

	// The best candidate has been chosen: it has the correct file, and its position 
	// is pretty much what we are looking for.
	// Compute effective position and data size.
	AKASSERT( in_uPosition >= pMemBlock->uPosition );
	AkUInt32 uPositionOffset = (AkUInt32)( in_uPosition - pMemBlock->uPosition );
	AkUInt32 uAvailableValidSize = pMemBlock->uAvailableSize - uPositionOffset;

	// Verify effective size and data address against alignment. Discard otherwise.
	/// NOTE: Suboptimal for last buffer. 
	if (   ( uAvailableValidSize <= io_uRequestedSize )				// Cannot overshoot desired size.
		// Alignment constraints. Relax requirements on size if last buffer.
		&& ( ( ( uAvailableValidSize % in_uRequiredAlign ) == 0 ) || ( in_bEof && uAvailableValidSize == io_uRequestedSize ) )
		&& ( ( ((AkUIntPtr)((AkUInt8*)pMemBlock->pData + uPositionOffset)) % in_uRequiredAlign ) == 0 )
		// ...or minimum size constraint.
		&& ( uAvailableValidSize >= in_uMinSize ) ) 
	{
		// Use this cache block.
		io_uRequestedSize = uAvailableValidSize;

		if ( pMemBlock->uRefCount == 0 )
		{
			// Free block. Pop it out of the free list.
			/// REVIEW Consider having an option to avoid searching cached data into free blocks. Would avoid this linear search.
			AKVERIFY( m_listFreeBuffers.Remove( pMemBlock ) == AK_Success );
			// pNextBlock is not used now that the block was dequeued, but it is an union with pTransfer, 
			// so we ought to clear it now.
			pMemBlock->pTransfer = NULL;
#ifndef AK_OPTIMIZED
			++m_uAllocs;
#endif
		}
		++pMemBlock->uRefCount;

		--m_uNumViewsAvail;
		AKASSERT( m_uNumViewsAvail >= m_listFreeBuffers.Length() );

		out_pMemBlock = pMemBlock;
		return uPositionOffset;
	}
	
	return 0;
}

// Comparison function for cache info binary search, using all keys. Returns 0 if match is perfect,
// -1 if a better match would be found before, 1 if a better match would be found after.
inline int Compare_AllKeys( const AkMemBlock * in_pSorted, const AkMemBlock * in_pSearched )
{
	// 1st key: file ID.
	if ( in_pSearched->fileID < in_pSorted->fileID )
		return -1;
	else if ( in_pSearched->fileID > in_pSorted->fileID )
		return 1;

	// 2nd key: position.
	else if ( in_pSearched->uPosition > in_pSorted->uPosition )
		return -1;
	else if ( in_pSearched->uPosition < in_pSorted->uPosition )
		return 1;

	// 3rd key: address
	else if ( (AkUIntPtr)in_pSearched->pData < (AkUIntPtr)in_pSorted->pData )
		return -1;
	else if ( (AkUIntPtr)in_pSearched->pData > (AkUIntPtr)in_pSorted->pData )
		return 1;

	// Perfect match.
	AKASSERT( in_pSearched->uPosition == in_pSorted->uPosition 
			&& in_pSearched->fileID == in_pSorted->fileID 
			&& in_pSearched->pData == in_pSorted->pData );
	return 0;
}

// Untag a block after a cancelled or failed IO transfer.
void CAkIOMemMgr::UntagBlock(
	AkMemBlock *	in_pMemBlock		// Memory block to tag with caching info.
	)
{
	AKASSERT( in_pMemBlock->IsTagged() );

	// Find block, remove, untag, reinsert.
	CHECK_CACHE_CONSISTENCY( in_pMemBlock );

#ifdef _DEBUG
	bool bFound = false;
#endif

	// Find the index of the block in the list, using all three keys.
	// Binary search: blocks are always sorted (file ID first, position second, buffer address third).
	AkInt32 iTop = 0, iBottom = m_arMemBlocks.Length()-1;
	AkInt32 iThis;
	do
	{
		iThis = ( iBottom - iTop ) / 2 + iTop; 
		int iCmp = Compare_AllKeys( IndexToMemBlock(m_arMemBlocks[iThis]), in_pMemBlock );
		if ( 0 == iCmp )
		{
#ifdef _DEBUG
			bFound = true;
#endif
			break;
		}
		else if ( iCmp < 0 )
			iBottom = iThis - 1;
		else
			iTop = iThis + 1;
	}
	while ( iTop <= iBottom );

	AKASSERT( bFound || !"Block wasn't found in list" );
	
	AkUInt32 uOriginalLocation = iThis;
	
	// Find the location where our "untagged" block should be inserted.
	// Need to use a temporary block with such info.
	AkMemBlock newBlockData = *in_pMemBlock;
	newBlockData.fileID = AK_INVALID_FILE_ID;

	iTop = 0;
	iBottom = m_arMemBlocks.Length()-1;
	do
	{
		iThis = ( iBottom - iTop ) / 2 + iTop; 
		int iCmp = Compare_AllKeys( IndexToMemBlock(m_arMemBlocks[iThis]), &newBlockData );
		if ( 0 == iCmp )
		{
			iBottom = iTop = iThis;
			break;
		}
		else if ( iCmp < 0 )
			iBottom = iThis - 1;
		else
			iTop = iThis + 1;
	}
	while ( iTop <= iBottom );

	// Insert at index either iBottom or iTop, depending on whether the location was found from above or below.
	AkUInt32 uNewLocation = AkMax( iBottom, iTop );
	
	m_arMemBlocks.Move( uOriginalLocation, uNewLocation );

	// Set info on actual block.
	in_pMemBlock->fileID			= AK_INVALID_FILE_ID;
	
	CHECK_CACHE_CONSISTENCY( in_pMemBlock );
}

// Untag all blocks (cache flush).
void CAkIOMemMgr::UntagAllBlocks()
{
	if ( m_bUseCache )
	{
		// Avoid touching the index dictionnary if block is not tagged.
		// 1st key of all blocks is set to INVALID, but ordering still needs to be updated
		// because of 2nd and 3rd keys. This is quite costly.
		AkUInt32 uNumBlocks = m_arMemBlocks.Length();
		for ( AkUInt32 uBlock = 0; uBlock < uNumBlocks; uBlock++ )
		{
			if ( m_pMemBlocks[uBlock].IsTagged() )
				UntagBlock( &m_pMemBlocks[uBlock] );
		}
		CHECK_CACHE_CONSISTENCY( NULL );
	}
}

// Tag a block with caching info before IO transfer.
void CAkIOMemMgr::TagBlock(
	AkMemBlock *	in_pMemBlock,		// Memory block to tag with caching info.
	CAkLowLevelTransfer * in_pTransfer,	// Associated transfer.
	AkFileID		in_fileID,			// File ID.
	AkUInt64		in_uPosition,		// Absolute position in file.
	AkUInt32		in_uDataSize		// Size of valid data fetched from Low-Level IO.
	)
{
	if ( !UseCache() )
	{
		// Not using cache. Blocks are never kept reordered. Just set data and leave.
		AKASSERT( in_pMemBlock->uRefCount == 1 );
		in_pMemBlock->uPosition			= in_uPosition;
		in_pMemBlock->uAvailableSize	= in_uDataSize;	
		in_pMemBlock->pTransfer			= in_pTransfer;
		return;
	}

	AKASSERT( !in_pMemBlock->pTransfer || !"Block already has transfer" );	// If you intend to tag this block, it should have been free.
	AKASSERT( in_pMemBlock->uRefCount == 1 );

	CHECK_CACHE_CONSISTENCY( in_pMemBlock );

#ifdef _DEBUG
	bool bFound = false;
#endif

	// Find the index of the block in the list, using all three keys.
	// Binary search: blocks are always sorted (file ID first, position second, buffer address third).
	AkInt32 iTop = 0, iBottom = m_arMemBlocks.Length()-1;
	AkInt32 iThis;
	do
	{
		iThis = ( iBottom - iTop ) / 2 + iTop; 
		int iCmp = Compare_AllKeys( IndexToMemBlock(m_arMemBlocks[iThis]), in_pMemBlock );
		if ( 0 == iCmp )
		{
#ifdef _DEBUG
			bFound = true;
#endif
			break;
		}
		else if ( iCmp < 0 )
			iBottom = iThis - 1;
		else
			iTop = iThis + 1;
	}
	while ( iTop <= iBottom );

	AKASSERT( bFound || !"Block wasn't found in list" );

	AkUInt32 uOriginalLocation = iThis;


	// Find the location where our block should be inserted.
	// Need to use a temporary block with such info.
	AkMemBlock newBlockData = *in_pMemBlock;
	newBlockData.fileID				= in_fileID;
	newBlockData.uPosition			= in_uPosition;

	iTop = 0;
	iBottom = m_arMemBlocks.Length()-1;
	do
	{
		iThis = ( iBottom - iTop ) / 2 + iTop; 
		int iCmp = Compare_AllKeys( IndexToMemBlock(m_arMemBlocks[iThis]), &newBlockData );
		if ( 0 == iCmp )
		{
			iBottom = iTop = iThis;
			break;
		}
		else if ( iCmp < 0 )
			iBottom = iThis - 1;
		else
			iTop = iThis + 1;
	}
	while ( iTop <= iBottom );

	// Insert at index either iBottom or iTop, depending on whether the location was found from above or below.
	AkUInt32 uNewLocation = AkMax( iBottom, iTop );

	m_arMemBlocks.Move( uOriginalLocation, uNewLocation );

	// Set info on actual block.
	in_pMemBlock->fileID			= in_fileID;
	in_pMemBlock->uPosition			= in_uPosition;
	in_pMemBlock->uAvailableSize	= in_uDataSize;	
	in_pMemBlock->pTransfer			= in_pTransfer;

	CHECK_CACHE_CONSISTENCY( in_pMemBlock );
}

// Temporary blocks.
// -----------------------------------
// Clone a memory block in order to have simultaneous low-level transfers on the 
// same mapped memory (needed by standard streams in deferred device). 
void CAkIOMemMgr::CloneTempBlock( 
	AkMemBlock * in_pMemBlockBase,
	AkMemBlock *& out_pMemBlock 
	)
{
	// Allocate a new block with values of in_pMemBlockBase.
	out_pMemBlock = (AkMemBlock*)AkAlloc( CAkStreamMgr::GetObjPoolID(), sizeof( AkMemBlock ) );
	if ( !out_pMemBlock )
		return;

	AkPlacementNew( out_pMemBlock ) AkMemBlock( in_pMemBlockBase->pData );
	out_pMemBlock->uAvailableSize = in_pMemBlockBase->uAvailableSize;
	out_pMemBlock->uPosition = in_pMemBlockBase->uPosition;
}

void CAkIOMemMgr::DestroyTempBlock( 
	AkMemBlock * in_pMemBlockBase,
	AkMemBlock * in_pMemBlock 
	)
{
	// Release block only if it is temporary.
	if ( in_pMemBlock != in_pMemBlockBase )
	{
		AkFree( CAkStreamMgr::GetObjPoolID(), in_pMemBlock );
	}
}

void CAkIOMemMgr::GetProfilingData(
	AkUInt32 in_uBlockSize,
	AkDeviceData &  out_deviceData
	)
{
#ifndef AK_OPTIMIZED
	out_deviceData.uMemSize = m_streamIOPoolSize;
	out_deviceData.uMemUsed = out_deviceData.uMemSize - ( in_uBlockSize * m_listFreeBuffers.Length() );
	out_deviceData.uAllocs	= m_uAllocs;
	out_deviceData.uFrees	= m_uFrees;
	if ( out_deviceData.uMemUsed > m_uPeakUsed )
		m_uPeakUsed = out_deviceData.uMemUsed;
	out_deviceData.uPeakUsed = m_uPeakUsed;
	out_deviceData.uNumViewsAvailable = m_uNumViewsAvail;
#endif
}


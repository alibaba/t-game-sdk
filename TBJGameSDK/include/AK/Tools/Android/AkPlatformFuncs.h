//////////////////////////////////////////////////////////////////////
//
// AkPlatformFuncs.h 
//
// Audiokinetic platform-dependent functions definition.
//
// Copyright (c) 2006 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <AK/Tools/Common/AkAssert.h>
#include <AK/SoundEngine/Common/AkTypes.h>

#include <time.h>
#include <stdlib.h>

namespace AKPLATFORM
{
	/// Platform Independent Helper
	inline void PerformanceFrequency( AkInt64 * out_piFreq )
	{
		// TO DO ANDROID ... is there something better
		*out_piFreq = CLOCKS_PER_SEC;
	}
}

#include <AK/Tools/POSIX/AkPlatformFuncs.h>

namespace AKPLATFORM
{

#ifndef AK_OPTIMIZED
	/// Output a debug message on the console (Ansi string)
	inline void OutputDebugMsg( const char* in_pszMsg )
	{
		// To see output of this 
		// adb logcat ActivityManager:I YourApplication:D AKDEBUG:D *:S
		//__android_log_print(ANDROID_LOG_INFO, "AKDEBUG", "%s", in_pszMsg);	
	}	
#else	
	inline void OutputDebugMsg( const char* ){}
#endif
	// Atomic Operations
    // ------------------------------------------------------------------

	/// Platform Independent Helper
	inline AkInt32 AkInterlockedIncrement( AkInt32 * pValue )
	{
		return __atomic_add_fetch(pValue,1, __ATOMIC_SEQ_CST);
	}

	/// Platform Independent Helper
	inline AkInt32 AkInterlockedDecrement( AkInt32 * pValue )
	{
		return __atomic_sub_fetch(pValue,1, __ATOMIC_SEQ_CST);
	}

	inline bool AkInterlockedCompareExchange( volatile AkInt32* io_pDest, AkInt32 in_newValue, AkInt32 in_expectedOldVal )
	{
		return __atomic_compare_exchange(io_pDest, &in_expectedOldVal, &in_newValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	}
	
	inline bool AkInterlockedCompareExchange( volatile AkInt64* io_pDest, AkInt64 in_newValue, AkInt64 in_expectedOldVal )
	{
		return __atomic_compare_exchange(io_pDest, &in_expectedOldVal, &in_newValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	}

	inline void AkMemoryBarrier()
	{
		 __atomic_thread_fence(__ATOMIC_SEQ_CST);
		 /*
#ifdef AK_CPU_ARM
		__asm("DMB ST");	//Wait for stores to complete.
#elif AK_CPU_X86
		__asm("mfence");	
#endif*/
	}

    // Time functions
    // ------------------------------------------------------------------

	/// Platform Independent Helper
    inline void PerformanceCounter( AkInt64 * out_piLastTime )
	{
		*out_piLastTime = clock();
	}


	template<class destType, class srcType>
	inline size_t AkSimpleConvertString( destType* in_pdDest, const srcType* in_pSrc, size_t in_MaxSize, size_t destStrLen(const destType *),  size_t srcStrLen(const srcType *) )
	{ 
		size_t i;
		size_t lenToCopy = srcStrLen(in_pSrc);
		
		lenToCopy = (lenToCopy > in_MaxSize-1) ? in_MaxSize-1 : lenToCopy;
		for(i = 0; i < lenToCopy; i++)
		{
			in_pdDest[i] = (destType) in_pSrc[i];
		}
		in_pdDest[lenToCopy] = (destType)0;
		
		return lenToCopy;
	}

	#define CONVERT_UTF16_TO_CHAR( _astring_, _charstring_ ) \
		_charstring_ = (char*)AkAlloca( (1 + AKPLATFORM::AkUtf16StrLen((const AkUtf16*)_astring_)) * sizeof(char) ); \
		AK_UTF16_TO_CHAR( _charstring_, (const AkUtf16*)_astring_, AKPLATFORM::AkUtf16StrLen((const AkUtf16*)_astring_)+1 ) 

	#define AK_UTF16_TO_OSCHAR(	in_pdDest, in_pSrc, in_MaxSize )	AKPLATFORM::AkSimpleConvertString(	in_pdDest, in_pSrc, in_MaxSize, strlen, AKPLATFORM::AkUtf16StrLen )
	#define AK_UTF16_TO_CHAR(	in_pdDest, in_pSrc, in_MaxSize )	AKPLATFORM::AkSimpleConvertString(	in_pdDest, in_pSrc, in_MaxSize, strlen, AKPLATFORM::AkUtf16StrLen )
	#define AK_CHAR_TO_UTF16(	in_pdDest, in_pSrc, in_MaxSize )	AKPLATFORM::AkSimpleConvertString(	in_pdDest, in_pSrc, in_MaxSize, AKPLATFORM::AkUtf16StrLen, strlen)	
	#define AK_OSCHAR_TO_UTF16(	in_pdDest, in_pSrc, in_MaxSize )	AKPLATFORM::AkSimpleConvertString(	in_pdDest, in_pSrc, in_MaxSize, AKPLATFORM::AkUtf16StrLen, strlen)	
	
	/// Stack allocations.
	#define AkAlloca( _size_ ) __builtin_alloca( _size_ )

	/// Platform Independent Helper
	inline void AkCreateThread( 
		AkThreadRoutine pStartRoutine,					// Thread routine.
		void * pParams,									// Routine params.
		const AkThreadProperties & in_threadProperties,	// Properties. NULL for default.
		AkThread * out_pThread,							// Returned thread handle.
		const char * /*in_szThreadName*/ )				// Opt thread name.
	{
		AKASSERT( out_pThread != NULL );

		pthread_attr_t  attr;

		// Create the attr
		AKVERIFY(!pthread_attr_init(&attr));
		// Set the stack size
		AKVERIFY(!pthread_attr_setstacksize(&attr,in_threadProperties.uStackSize));

		AKVERIFY(!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE));		

		// Create the tread
		int     threadError = pthread_create( out_pThread, &attr, pStartRoutine, pParams);
		AKASSERT( threadError == 0 );
		AKVERIFY(!pthread_attr_destroy(&attr));

		if( threadError != 0 )
		{
			AkClearThread( out_pThread );
			return;
		}

		// ::CreateThread() return NULL if it fails.
		if ( !*out_pThread )
		{
			AkClearThread( out_pThread );
			return;
		}		

		// Try to set the thread policy
		int sched_policy = in_threadProperties.uSchedPolicy;		

		// Get the priority for the policy
		int minPriority, maxPriority;
		minPriority = sched_get_priority_min(sched_policy);
		maxPriority = sched_get_priority_max(sched_policy);

		// Set the thread priority if valid
		sched_param schedParam;
		schedParam.sched_priority = in_threadProperties.nPriority;	
		AKASSERT( in_threadProperties.nPriority >= minPriority && in_threadProperties.nPriority <= maxPriority );		

		//pthread_setschedparam WILL fail on Android Lollipop when used with SCHED_FIFO (the default).  Not allowed anymore. (ignore the error code).
		int err = pthread_setschedparam(*out_pThread, sched_policy, &schedParam);
		if (err != 0)
		{			
			//Make sure the priority is well set, even if the policy could not.			
			//sched_policy = SCHED_NORMAL;
			minPriority = sched_get_priority_min(sched_policy);
			maxPriority = sched_get_priority_max(sched_policy);
			if (in_threadProperties.nPriority == AK_THREAD_PRIORITY_ABOVE_NORMAL)
				schedParam.sched_priority = maxPriority;
			else if (in_threadProperties.nPriority == AK_THREAD_PRIORITY_BELOW_NORMAL)
				schedParam.sched_priority = minPriority;
			else
				schedParam.sched_priority = (maxPriority + minPriority) / 2;
			err = pthread_setschedparam(*out_pThread, sched_policy, &schedParam);						
			AKASSERT(err == 0);
		}
	}
}
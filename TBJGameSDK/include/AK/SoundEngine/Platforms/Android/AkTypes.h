//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

// AkTypes.h

/// \file 
/// Data type definitions.

#pragma once

#define AK_ANDROID
#define AK_LFECENTER							///< Internal use
#define AK_REARCHANNELS							///< Internal use

#if defined(__LP64__) || defined(_LP64)
#ifdef __aarch64__
#define AK_CPU_ARM_64	
#else
#define AK_CPU_X86_64
#endif
#else
#ifdef __arm__
#define AK_CPU_ARM
#else
//#define AK_CPU_X86	//To enable when SIMD defines are enabled.
#endif
#endif

#include <AK/SoundEngine/Platforms/POSIX/AkTypes.h>
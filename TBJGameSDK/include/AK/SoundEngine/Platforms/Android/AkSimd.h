//////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011 Audiokinetic Inc. / All Rights Reserved
//
//////////////////////////////////////////////////////////////////////

// AkSimd.h

/// \file 
/// AKSIMD - Android implementation


#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

#define AKSIMD_GETELEMENT_V4F32( __vName, __num__ )			((float*)&(__vName))[(__num__)]							///< Retrieve scalar element from vector.
#define AKSIMD_GETELEMENT_V2F32( __vName, __num__ )			((float*)&(__vName))[(__num__)]							///< Retrieve scalar element from vector.
#define AKSIMD_GETELEMENT_V4I32( __vName, __num__ )			((int*)&(__vName))[(__num__)]							///< Retrieve scalar element from vector.

#ifdef AK_CPU_ARM_NEON
#include <AK/SoundEngine/Platforms/arm_neon/AkSimd.h>
#else
#include <AK/SoundEngine/Platforms/Generic/AkSimd.h>
#endif


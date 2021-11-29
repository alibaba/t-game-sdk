// IntegrationDemo.h
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Declares various helpers for the Integration Demo

#pragma once
#include <string>
#include <AK/SoundEngine/Common/AkTypes.h>

namespace IntegrationDemoHelpers
{
	/// Returns the number of elements in an array
	template <class T, size_t N> AkForceInline size_t AK_ARRAYSIZE(T(&)[N])
	{
		return N;
	}

	void WordWrapString( std::string& io_strWrapped, const size_t in_nMaxWidth );
	std::string WordWrapString( const char* in_szOriginal, const size_t in_nMaxWidth );
}

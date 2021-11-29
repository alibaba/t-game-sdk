// Copyright (C) 2010 Audiokinetic Inc 

#pragma once

#if defined AK_XBOXONE || defined AK_PS4

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include "Page.h"
#include <string>

/// Class representing the Non-recordable background music Demo page.  This pages demonstrates how to setup the background music
/// in your game and project to make sure it is not recorded by the DVR, thus compliying with the TCRs about licensed music.
class DemoBGMusic : public Page
{
public:

	/// DemoBGMusic class constructor
	DemoBGMusic( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Override of the Page::Draw() method.
	virtual void Draw();
	void InitControls();
	
private:

	void Recordable_Pressed( void*, ControlEvent* );
	void NonRecordable_Pressed( void*, ControlEvent* );

	bool m_bPlayLicensed;
	bool m_bPlayCopyright;
};

#endif
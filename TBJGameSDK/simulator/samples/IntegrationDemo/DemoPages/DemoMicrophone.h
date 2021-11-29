// DemoMicrophone.h
// Copyright (C) 2010 Audiokinetic Inc 
/// \file
/// Contains the declaration for the DemoMicrophone class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include "Page.h"
#include "SoundInput.h"


/// Class representing the Markers Demo page. This page demonstrates how to use the markers
/// in a wav file to synchronize events in the game with the audio being played.
class DemoMicrophone : public Page
{
public:

	/// DemoMicrophone class constructor
	DemoMicrophone( Menu& in_ParentMenu );

	virtual ~DemoMicrophone();

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Override of the Page::Draw() method.
	virtual void Draw();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();
	void PlayMicroButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );
	void DelayButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );
	void StartRecording();
	void StopRecording();

	SoundInput* m_pInput;
	bool m_bPlaying;
	bool m_bDelayed;
};

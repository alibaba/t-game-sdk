// DemoRTPCCarEngine.h
// Copyright (C) 2010 Audiokinetic Inc.
/// \file
/// Contains the declaration for the DemoRTPCCarEngine class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include "Page.h"


/// Class representing the RTPC Demo page. This page demonstrates how to use real-time parameter controls
/// to modify the audio being played. In this example, an "RPM" RTPC is set up to modify the sound of an
/// engine,
class DemoRTPCCarEngine : public Page
{
public:

	/// DemoRTPCCarEngine class constructor
	DemoRTPCCarEngine( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Delegate function for the "Start/Stop Engine" button.
	void PlayEngineButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "RPM" numeric control.
	void RPMNumeric_ValueChanged( void* in_pSender, ControlEvent* in_pEvent );

	/// Whether or not the car engine sound is currently playing.
	bool m_bPlayingEngine;

	/// Game Object ID for the "Car".
	static const AkGameObjectID GAME_OBJECT_CAR = 100;
};

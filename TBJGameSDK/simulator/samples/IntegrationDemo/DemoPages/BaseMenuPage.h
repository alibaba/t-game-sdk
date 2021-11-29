// BaseMenuPage.h
// Copyright (C) 2010 Audiokinetic Inc.
/// \file 
/// Defines the Base page that is loaded by the menu system when it is created.

#pragma once

#include "Page.h"


/// This class defines the first page that the Menu system automatically loads
/// when the Menu class is instantiated.
class BaseMenuPage : public Page
{
public:

	/// BaseMenuPage class constructor
	BaseMenuPage( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

	/// Override of the parent's update method.
	virtual bool Update();

private:

	/// Initializes the various components on the page.
	virtual void InitControls();

	/// Delegate function for the "Footsteps Demo" button.
	void OpenFootstepsDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "RTPC Demo" button.
	void OpenRTPCDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Dialogue Demos" button.
	void OpenDialogueDemosPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Markers Demo" button.
	void OpenMarkersDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Music Callbacks Demo" button.
	void OpenMusicCallbacksDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the " Interactive Music Demo" button.
	void OpenInteractiveMusicDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Motion Demo" button.
	void OpenMotionDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	void OpenMicrophoneDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Positioning Demo" button.
	void OpenPositioningDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "External Sources Demo" button.
	void OpenExternalSourcesDemoPage( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "RSX Demo" button.
	void OpenRSXDemoPage( void* in_pSender, ControlEvent* in_pEvent );

#if defined AK_XBOXONE || defined AK_PS4
	/// Delegate function for the "BGM Demo" button.
	void OpenBGMDemoPage( void* in_pSender, ControlEvent* in_pEvent );
#endif

	/// Delegate function for the "Exit" button.
	void ExitButton_OnPress( void* in_pSender, ControlEvent* in_pEvent );
};

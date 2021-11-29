// DemoLocalization.h
// Copyright (C) 2010 Audiokinetic Inc 
/// \file
/// Contains the declaration for the DemoLocalization class.

#pragma once

#include "Page.h"

/// This class represents the Localization Demo page. This page demonstrates how to implement
/// localized audio in your game. It uses the default low-level IO implementation included
/// with the Wwise SDK sample code.
class DemoLocalization : public Page
{
public:

	/// DemoLocalization class constructor
	DemoLocalization( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Delegate function for the "Say Hello" button.
	void HelloButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Language" toggle.
	void LanguageToggle_SelectionChanged( void* in_pSender, ControlEvent* in_pEvent );

	/// Game Object ID for the "Human".
	static const AkGameObjectID GAME_OBJECT_HUMAN = 100;
};

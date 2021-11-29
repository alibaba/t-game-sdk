// DemoDialogueMenu.h
// Copyright (C) 2010 Audiokinetic Inc 
/// \file
/// Contains the declaration for the DemoDialogueMenu class.

#pragma once

#include "Page.h"

/// This class represents the Dialogue Demos Menu page. This is simply a submenu to
/// link to various demonstrations involving dialogue.
class DemoDialogueMenu : public Page
{
public:

	/// DemoDialogueMenu class constructor
	DemoDialogueMenu( Menu& in_ParentMenu );

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Delegate function for the "Localization Demo" button.
	void LocalizationButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Dynamic Dialogue Demo" button.
	void DynamicDialogueButton_Pressed( void* in_pSender, ControlEvent* in_pEvent );
};

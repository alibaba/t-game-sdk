// DemoDialogueMenu.cpp
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Defines the methods declared in DemoDialogueMenu.h.

#include "stdafx.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

#include "Menu.h"
#include "DemoDialogueMenu.h"
#include "DemoDynamicDialogue.h"
#include "DemoLocalization.h"


/////////////////////////////////////////////////////////////////////
// DemoDialogueMenu Public Methods
/////////////////////////////////////////////////////////////////////

DemoDialogueMenu::DemoDialogueMenu( Menu& in_ParentMenu ):Page( in_ParentMenu, "Dialogue Demos Menu" )
{

}


/////////////////////////////////////////////////////////////////////
// DemoDialogueMenu Private Methods
/////////////////////////////////////////////////////////////////////

void DemoDialogueMenu::InitControls()
{
	ButtonControl* newBtn;

	// Add button to open the localization demo page
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Localization Demo" );
	newBtn->SetDelegate( (PageMFP)&DemoDialogueMenu::LocalizationButton_Pressed );
	m_Controls.push_back( newBtn );

	// Add button to open the dynamic dialogue demo page
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Dynamic Dialogue Demo" );
	newBtn->SetDelegate( (PageMFP)&DemoDialogueMenu::DynamicDialogueButton_Pressed );
	m_Controls.push_back( newBtn );
}

void DemoDialogueMenu::LocalizationButton_Pressed( void*, ControlEvent* )
{
	DemoLocalization* pg = new DemoLocalization( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

void DemoDialogueMenu::DynamicDialogueButton_Pressed( void*, ControlEvent* )
{
	DemoDynamicDialogue* pg = new DemoDynamicDialogue( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

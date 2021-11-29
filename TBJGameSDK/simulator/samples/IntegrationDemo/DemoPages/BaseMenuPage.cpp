// BaseMenuPage.cpp
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Defines the methods declared in BaseMenuPage.h

#include"stdafx.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

#include "BaseMenuPage.h"
#include "MenuPages.h"
#include "Menu.h"

/////////////////////////////////////////////////////////////////////
// BaseMenuPage Public Methods
/////////////////////////////////////////////////////////////////////

BaseMenuPage::BaseMenuPage( Menu& in_ParentMenu ):Page( in_ParentMenu, "Wwise Integration Demo" )
{
	m_szHelp  = "Welcome to the Wwise Integration Demonstration! This "
				"application contains samples that demonstrate how to "
				"integrate various Wwise features and concepts into a "
				"game.\n\n"
				"Navigation:\n"
				"  - UP / DOWN on the <<DIRECTIONAL_TYPE>>: move between items on the page.\n"
				"  - <<ACTION_BUTTON>>: activate the selected item.\n"
				"  - <<BACK_BUTTON>>: go back a page.\n\n"
				"Controls:\n"
				"  Certain controls (toggles, sliders) allow you to "
				"change values. Press LEFT/RIGHT on the <<DIRECTIONAL_TYPE>> "
				"to change these controls' values.\n\n"
				"Pressing <<HELP_BUTTON>> at any time will display help for the current page.\n";
}

bool BaseMenuPage::Init()
{
	// Load the Init sound bank
	// NOTE: The Init sound bank must be the first bank loaded by Wwise!
	AkBankID bankID;
	if ( AK::SoundEngine::LoadBank( "Init.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Init.bnk" );
		return false;
	}

	// Initialize the page
	return Page::Init();
}

void BaseMenuPage::Release()
{
	// Unload the init soundbank
	AK::SoundEngine::UnloadBank( "Init.bnk", NULL );

	// Release the page
	Page::Release();
}

bool BaseMenuPage::Update()
{
	// Avoid returning false when the "back" button is pressed, but return false if an error occurs!
	Page::Update();
	return ! ErrorOccured();
}

void BaseMenuPage::InitControls()
{
	ButtonControl* newBtn;

	// Add button linking to the Dialogue Demos menu
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Dialogue Demos" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenDialogueDemosPage );
	m_Controls.push_back( newBtn );
	
	// Add button linking to RTPC Demo (Car Engine)
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "RTPC Demo (Car Engine)" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenRTPCDemoPage );
	m_Controls.push_back( newBtn );
	
	// Add button linking to Footsteps demo
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Footsteps Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenFootstepsDemoPage );
	m_Controls.push_back( newBtn );

	// Add button linking to Markers demo
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Subtitles/Markers Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenMarkersDemoPage );
	m_Controls.push_back( newBtn );

	// Add button linking to Music Callbacks demo
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Music Callbacks Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenMusicCallbacksDemoPage );
	m_Controls.push_back( newBtn );

	// Add button linking to Interactive Music demo
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Interactive Music Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenInteractiveMusicDemoPage );
	m_Controls.push_back( newBtn );

#if defined AK_MOTION
	// Add button linking to Motion demo
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Motion Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenMotionDemoPage );
	m_Controls.push_back( newBtn );
#endif //defined AK_MOTION

#if defined INTEGRATIONDEMO_MICROPHONE
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Microphone Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenMicrophoneDemoPage );
	m_Controls.push_back( newBtn );
#endif // defined INTEGRATIONDEMO_MICROPHONE

	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Positioning Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenPositioningDemoPage );
	m_Controls.push_back( newBtn );

	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "External Sources Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenExternalSourcesDemoPage );
	m_Controls.push_back( newBtn );

#if defined AK_PS3
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "RSX Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenRSXDemoPage );
	m_Controls.push_back( newBtn );
#endif // AK_PS3

#if defined AK_XBOXONE || defined AK_PS4
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Non-recordable Music Demo" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::OpenBGMDemoPage );
	m_Controls.push_back( newBtn );
#endif
	//TODO: add other demo links here

	// Add an Exit button
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Exit" );
	newBtn->SetDelegate( (PageMFP)&BaseMenuPage::ExitButton_OnPress );
	m_Controls.push_back( newBtn );
}


/////////////////////////////////////////////////////////////////////
// BaseMenuPage Private Methods
/////////////////////////////////////////////////////////////////////

void BaseMenuPage::OpenFootstepsDemoPage( void*, ControlEvent* )
{
	DemoFootstepsManyVariables* pg = new DemoFootstepsManyVariables( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

void BaseMenuPage::OpenRTPCDemoPage( void*, ControlEvent* )
{
	DemoRTPCCarEngine* pg = new DemoRTPCCarEngine( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

void BaseMenuPage::OpenDialogueDemosPage( void*, ControlEvent* )
{
	DemoDialogueMenu* pg = new DemoDialogueMenu( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

void BaseMenuPage::OpenMarkersDemoPage( void*, ControlEvent* )
{
	DemoMarkers* pg = new DemoMarkers( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

void BaseMenuPage::OpenMusicCallbacksDemoPage( void*, ControlEvent* )
{
	DemoMusicCallbacksRoot* pg = new DemoMusicCallbacksRoot( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

void BaseMenuPage::OpenInteractiveMusicDemoPage( void*, ControlEvent* )
{
	DemoInteractiveMusic* pg = new DemoInteractiveMusic( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

#if defined AK_MOTION
void BaseMenuPage::OpenMotionDemoPage( void*, ControlEvent* )
{
	DemoMotion* pg = new DemoMotion( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}
#endif // AK_MOTION

#if defined INTEGRATIONDEMO_MICROPHONE
void BaseMenuPage::OpenMicrophoneDemoPage( void*, ControlEvent* )
{
	DemoMicrophone* pg = new DemoMicrophone( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}
#endif // defined INTEGRATIONDEMO_MICROPHONE

void BaseMenuPage::OpenPositioningDemoPage( void*, ControlEvent* )
{
	DemoPositioning* pg = new DemoPositioning( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

void BaseMenuPage::OpenExternalSourcesDemoPage( void*, ControlEvent* )
{
	DemoExternalSources* pg = new DemoExternalSources( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}

#if defined AK_PS3
void BaseMenuPage::OpenRSXDemoPage( void*, ControlEvent* )
{
	DemoRSX* pg = new DemoRSX( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}
#endif // AK_PS3

void BaseMenuPage::ExitButton_OnPress( void*, ControlEvent* )
{
	m_pParentMenu->QuitApplication();
}

#if defined AK_XBOXONE || defined AK_PS4
void BaseMenuPage::OpenBGMDemoPage( void* in_pSender, ControlEvent* in_pEvent )
{
	DemoBGMusic* pg = new DemoBGMusic( *m_pParentMenu );
	m_pParentMenu->StackPage( pg );
}
#endif

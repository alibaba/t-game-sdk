
// Copyright (C) 2010 Audiokinetic Inc
// DemoPositioning.cpp
/// \file
/// Defines all methods declared in DemoPositioning.h

#include "stdafx.h"

#include <math.h>
#include "Menu.h"
#include "MovableChip.h"
#include "DemoPositioning.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

//If you get a compiling error here, it means the file wasn't generated with the banks.  Did you generate the soundbanks before compiling?
#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		

//Our game object ID.  Completely arbitrary.
#define GAME_OBJECT_POSTEST 100

/////////////////////////////////////////////////////////////////////
// DemoPositioning Public Methods
/////////////////////////////////////////////////////////////////////

DemoPositioning::DemoPositioning( Menu& in_ParentMenu )
	: Page( in_ParentMenu, "Positioning Demo")
	, m_pChip( NULL )
	, m_fGameObjectX( 0 )
	, m_fGameObjectZ( 0 )
	, m_fWidth( 0.0f )
	, m_fHeight( 0.0f )

{
	m_szHelp =  "This demo shows how to do 3D positioning in "
				"Wwise. A helicopter sound starts playing as soon "
				"as you enter the page. Move the 'o' around "
                "in X and Z, i.e., the plane of the screen, using the "
				"following keys: <<UG_RIGHT_STICK>> <<DIRECTIONAL_TYPE>>,"
				"and hear the sound move along with it. "
				"Coordinates are displayed at the bottom-left of the screen.";
}

bool DemoPositioning::Init()
{
	// Register the "Human" game object
	AK::SoundEngine::RegisterGameObj( GAME_OBJECT_POSTEST, "Positioning Demo" );

	// Load the sound bank
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "Positioning_Demo.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Positioning_Demo.bnk" );
		return false;
	}

	AK::SoundEngine::PostEvent( "Play_Positioning_Demo", GAME_OBJECT_POSTEST );

	return Page::Init();
}

void DemoPositioning::Release()
{
	AK::SoundEngine::StopAll();
	AK::SoundEngine::UnregisterGameObj( GAME_OBJECT_POSTEST );
	AK::SoundEngine::UnloadBank( "Positioning_Demo.bnk", NULL );

	Page::Release();
}

void DemoPositioning::UpdateGameObjPos()
{
	float x, y;
	m_pChip->GetPos(x, y);

	// Set 3D position
	AkSoundPosition soundPos;
	m_fGameObjectX = soundPos.Position.X = ( ( x / m_fWidth ) - 0.5f ) * 200.0f;
	soundPos.Position.Y = 0;
	m_fGameObjectZ = soundPos.Position.Z = -( ( y / m_fHeight ) - 0.5f ) * 200.0f;;
	soundPos.Orientation.Z = 1;
	soundPos.Orientation.Y = soundPos.Orientation.X = 0;
	AK::SoundEngine::SetPosition( GAME_OBJECT_POSTEST, soundPos );
}

bool DemoPositioning::Update()
{
	//Always update the MovableChip

	bool bMoved = false;
	UniversalInput::Iterator it;
	for ( it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++ )
	{
		// Skip this input device if it's not connected
		if ( ! it->IsConnected() )
			continue;

		m_pChip->Update(*it);

		bMoved = true;
	}

	if ( bMoved )
	{
		UpdateGameObjPos();
	}

	return Page::Update();
}

void DemoPositioning::Draw()
{
	Page::Draw();

	m_pChip->Draw();

	char strBuf[50];
	snprintf( strBuf, 50, "X: %.2f\nZ: %.2f", m_fGameObjectX, m_fGameObjectZ );

	static int s_nOffset = 2 * GetLineHeight( DrawStyle_Text );

	DrawTextOnScreen( strBuf, 5, m_pParentMenu->GetHeight() - s_nOffset, DrawStyle_Text );

	// Display instructions at the bottom of the page
	int iInstructionsY = m_pParentMenu->GetHeight() - 3 * GetLineHeight( DrawStyle_Text );
	DrawTextOnScreen( "(Press <<UG_BUTTON2>> To Go Back...)", m_pParentMenu->GetWidth() / 4, iInstructionsY, DrawStyle_Text );
}

bool DemoPositioning::OnPointerEvent( PointerEventType in_eType, int in_x, int in_y )
{
	if ( in_eType == PointerEventType_Moved )
	{
		m_pChip->SetPos( (float) in_x, (float) in_y );
		UpdateGameObjPos();
	}

	return Page::OnPointerEvent( in_eType, in_x, in_y );
}

void DemoPositioning::InitControls()
{
	m_pChip = new MovableChip(*this);
	m_pChip->SetLabel( "o" );
	m_pChip->UseJoystick(UG_STICKRIGHT);
	m_pChip->SetNonLinear();

	m_fWidth = (float)m_pParentMenu->GetWidth() - (float)m_pChip->GetRightBottomMargin();
	m_fHeight = (float)m_pParentMenu->GetHeight() - (float)m_pChip->GetRightBottomMargin();
}


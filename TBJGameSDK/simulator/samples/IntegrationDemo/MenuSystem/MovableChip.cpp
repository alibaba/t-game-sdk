// Copyright (C) 2010 Audiokinetic Inc

#include "stdafx.h"
#include "MovableChip.h"
#include "Page.h"
#include "Menu.h"
#include <float.h>
#include <math.h>

// balary todo use vstudio define
#if defined AK_PS3 || defined AK_WII_FAMILY || defined AK_VITA || defined AK_PS4  || defined AK_3DS || defined AK_IOS || defined AK_MAC_OS_X || defined AK_ANDROID || defined AK_LINUX
#define _copysign copysign
#endif

#define MARGIN 15

MovableChip::MovableChip(Page& in_rParentPage) : Control(in_rParentPage)
{
	m_x = ( in_rParentPage.ParentMenu()->GetWidth() - MARGIN ) / 2.0f;
	m_y = ( in_rParentPage.ParentMenu()->GetHeight() - MARGIN ) / 2.0f;
	m_bNonLinear = false;
	m_fMaxSpeed = 5;
}

MovableChip::~MovableChip()
{
}

/// Updates the Control based on the input received.
/// \sa UGBtnState

void MovableChip::Update( const UniversalGamepad& in_Gamepad )
{
	//Move the position according to the joystick increments.
	float xPos = 0;
	float yPos = 0;
	in_Gamepad.GetStickPosition(m_Stick, xPos, yPos);

	//Dead zone
	if(fabs(xPos) < 0.1f)
		xPos = 0;
	if(fabs(yPos) < 0.1f)
		yPos = 0;

	if (m_bNonLinear)
	{
		float xSign = (float)_copysign(1.0f, xPos);
		float ySign = (float)_copysign(1.0f, yPos);

		xPos = (pow(20.0f, fabs(xPos)) - 1) / 19 * xSign;
		yPos = (pow(20.0f, fabs(yPos)) - 1) / 19 * ySign;
	}

	
	//Use the dpad
	if( in_Gamepad.IsButtonDown( UG_DPAD_DOWN ) )
	{
		yPos = -1;
	}
	if( in_Gamepad.IsButtonDown( UG_DPAD_UP ) )
	{
		yPos = 1;
	}
	if( in_Gamepad.IsButtonDown( UG_DPAD_LEFT ) )
	{
		xPos = -1;
	}
	if( in_Gamepad.IsButtonDown( UG_DPAD_RIGHT ) )
	{
		xPos = 1;
	}
	
	m_x += xPos * m_fMaxSpeed;
	m_y += -yPos * m_fMaxSpeed;

	//Clamp the position within the screen
	if (m_x > m_pParentPage->ParentMenu()->GetWidth() - MARGIN)
		m_x = (float)m_pParentPage->ParentMenu()->GetWidth() - MARGIN;

	if (m_y > m_pParentPage->ParentMenu()->GetHeight() - MARGIN)
		m_y = (float)m_pParentPage->ParentMenu()->GetHeight() - MARGIN;

	if (m_x < 0)
		m_x = 0;

	if (m_y < 0)
		m_y = 0;

	m_iXPos = (int)m_x;
	m_iYPos = (int)m_y;
}

/// Draws the Control to the screen.
/// \sa DrawStyle
void MovableChip::Draw( DrawStyle in_eDrawStyle )
{
	DrawTextOnScreen( m_szLabel.c_str(), m_iXPos, m_iYPos, in_eDrawStyle );
}

void MovableChip::UseJoystick(UGThumbstick in_StickToUse)
{
	m_Stick = in_StickToUse;
}

int MovableChip::GetRightBottomMargin() const
{
	return MARGIN;
}

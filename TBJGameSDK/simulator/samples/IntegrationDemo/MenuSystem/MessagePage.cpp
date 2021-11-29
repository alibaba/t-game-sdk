// MessagePage.cpp
// Copyright (C) 2010 Audiokinetic Inc
/// \file
/// Defines all methods declared in MessagePage.h

#include "stdafx.h"

#include "Menu.h"
#include "MessagePage.h"

/////////////////////////////////////////////////////////////////////
// ErrorDisplayPage Public Methods
/////////////////////////////////////////////////////////////////////

MessagePage::MessagePage( Menu& in_ParentMenu ):Page( in_ParentMenu, "--Message--" )
{
	m_MsgDrawStyle = DrawStyle_Text;
}

bool MessagePage::Update()
{
	UniversalInput::Iterator it;
	for ( it = m_pParentMenu->Input()->Begin(); it != m_pParentMenu->Input()->End(); it++ )
	{
		// Check if the user wants to go "Back"
		if ( it->IsButtonTriggered( UG_BUTTON2 ) )
		{
			return false;
		}
	}

	return true;
}

void MessagePage::Draw()
{
	int iHeight = m_pParentMenu->GetHeight();
	int iWidth  = m_pParentMenu->GetWidth();

	// Draw the page title
	DrawTextOnScreen( m_szPageTitle.c_str(), iWidth / 10, iHeight / 10, DrawStyle_Title );

	// Display the message
	DrawTextOnScreen( m_szMessage.c_str(), iWidth / 10, iHeight / 4, m_MsgDrawStyle );

	// Display instructions at the bottom of the page
	int iInstructionsY = iHeight - 2 * GetLineHeight( DrawStyle_Text );
	DrawTextOnScreen( "(Press <<UG_BUTTON2>> To Go Back...)", iWidth / 4, iInstructionsY, DrawStyle_Text );
}

void MessagePage::SetMessage( const char* in_szMessage, DrawStyle in_eDrawStyle )
{
	m_szMessage = in_szMessage;
	m_MsgDrawStyle = in_eDrawStyle;
}

/////////////////////////////////////////////////////////////////////
// ErrorDisplayPage Private Methods
/////////////////////////////////////////////////////////////////////

void MessagePage::InitControls()
{
	// No controls on this page..
}

// MessagePage.h
// Copyright (C) 2010 Audiokinetic Inc
/// \file
/// Contains the declaration for the MessagePage class.

#pragma once

#include "Page.h"

class MessagePage : public Page
{
public:

	/// MessagePage class constructor.
	MessagePage(
		Menu& in_pParentMenu	///< - Pointer to the Menu that the page belongs to
		);

	/// Override of the Parent's Update() method.
	virtual bool Update();

	/// Override of the Parent's Draw() method.
	virtual void Draw();

	/// Sets the error message to display.
	virtual void SetMessage( const char* in_szMessage, DrawStyle in_eDrawStyle = DrawStyle_Text );

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// The error message to be displayed.
	std::string m_szMessage;

	/// The message's style (text, error, etc..)
	DrawStyle m_MsgDrawStyle;
};

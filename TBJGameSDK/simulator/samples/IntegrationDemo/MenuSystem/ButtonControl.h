// ButtonControl.h
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Contains the declaration of the Button Control type.

#pragma once

#include "Control.h"


/// This class represents a simple button control on a page in the Menu system.
/// The button can be activated to trigger its delegate function.
class ButtonControl : public Control
{
public:
	
	/// ButtonControl class constructor. Accepts a reference to the Page which contains the control.
	ButtonControl( 
		Page& in_pParentPage	///< - Pointer to the Page that contains the button
		);

	/// Updates the button based on the given input. If the button is activated, calls the delegate function.
	virtual void Update( 
		const UniversalGamepad& in_Gamepad	///< - The input to react to
		);

	/// Draws the button to the screen with the desired DrawStyle.
	/// \sa DrawStyle
	virtual void Draw( 
		DrawStyle in_eDrawStyle = DrawStyle_Control		///< - The style with which to draw the button (defaults to "DrawStyle_Control")
		);

	/// Called when the control goes into focus.
	virtual void OnFocus();

	/// Called when the control loses focus.
	virtual void OnLoseFocus();

	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );
};

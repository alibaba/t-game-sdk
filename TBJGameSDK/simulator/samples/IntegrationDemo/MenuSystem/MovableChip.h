// Copyright (C) 2010 Audiokinetic Inc

#pragma once
#include "Control.h"


/// This control moves around in the screen using a joystick
class MovableChip : public Control
{
public:
	MovableChip(Page& in_pParentPage);
	virtual ~MovableChip();

	/// Updates the Control based on the input received.
	/// \sa UGBtnState
	virtual void Update( const UniversalGamepad& in_Gamepad	);

	/// Draws the Control to the screen.
	/// \sa DrawStyle
	virtual void Draw( DrawStyle in_eDrawStyle = DrawStyle_Control );

	void UseJoystick(UGThumbstick in_StickToUse);

	void GetPos(float &x, float &y) {x = m_x; y = m_y;}
	void SetPos(float x, float y) {m_x = x; m_y = y;}
	void SetMaxSpeed(float in_fSpeed) {m_fMaxSpeed = in_fSpeed;}
	void SetNonLinear() {m_bNonLinear = true;}

	int GetRightBottomMargin() const;

private:
	UGThumbstick m_Stick;
	float m_fMaxSpeed;
	float m_x;
	float m_y;
	bool m_bNonLinear;
};

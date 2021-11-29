// DemoPositioning.h
// Copyright (C) 2010 Audiokinetic Inc
/// \file
/// Contains the declaration for the DemoPositioning class.

#pragma once

#include "Page.h"
#include "MovableChip.h"

class DemoPositioning : public Page
{
public:

	/// DemoPositioning class constructor.
	DemoPositioning(
		Menu& in_pParentMenu	///< - Pointer to the Menu that the page belongs to
		);

	/// Override of the Parent's Init() method.
	virtual bool Init();

	/// Override of the Parent's Release() method.
	virtual void Release();

	/// Override of the Parent's Update() method.
	virtual bool Update();

	/// Override of the Parent's Draw() method.
	virtual void Draw();

	virtual bool OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

private:

	/// Initializes the controls on the page.
	virtual void InitControls();
	void UpdateGameObjPos();

	MovableChip *m_pChip;

	float m_fGameObjectX;
	float m_fGameObjectZ;
	float m_fWidth;
	float m_fHeight;
};

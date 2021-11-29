
// DemoFootstepsManyVariables.h
// Copyright (C) 2010 Audiokinetic Inc
/// \file
/// Contains the declaration for the DemoFootstepsManyVariables class.

#pragma once

#include "Page.h"
#include "MovableChip.h"

#ifndef AK_VITA
	#define _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING
#endif // AK_VITA


class DemoFootstepsManyVariables : public Page
{
public:

	/// DemoFootstepsManyVariables class constructor.
	DemoFootstepsManyVariables(
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

	/// Delegate function for the "m_ctrlWeight" Numeric
	void Weight_ValueChanged( void* in_pSender, ControlEvent* in_pEvent );
	void ManageSurfaces(int x, int y, int in_GameObject);
	void ManageEnvironement(int x, int y, int in_GameObject);
	void UpdateGameObjPos();

#if defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )
	int ComputeUsedBankMask(int x, int y);
#endif // defined( _DEMOFOOTSTEPS_DYNAMIC_BANK_LOADING )

	MovableChip *m_pChip;
	
	float m_weight;
	float m_LastX;
	float m_LastY;
	int m_maskCurrentBanks;
	int m_iSurface;
	int m_iLastFootstepTick;
};

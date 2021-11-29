// DemoExternalSources.h
// Copyright (C) 2011 Audiokinetic Inc 
/// \file
/// Contains the declaration for the DemoExternalSources class.

#pragma once

#include "Page.h"

/// This class represents the External Sources Demo page. This page demonstrates how to use 
/// external sources in your game. 
class DemoExternalSources : public Page
{
public:

	/// DemoExternalSources class constructor
	DemoExternalSources( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Delegate function for the "Play 1-2-3" button.
	void Play123_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Play 4-5-6" button.
	void Play456_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Game Object ID for the "Human".
	static const AkGameObjectID GAME_OBJECT_HUMAN = 100;

	/// File package ID. This file package contains all the external sources.
	AkUInt32	m_uPackageID;
};

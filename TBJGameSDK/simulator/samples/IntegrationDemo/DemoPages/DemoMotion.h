// DemoMotion.h
// Copyright (C) 2010 Audiokinetic Inc
/// \file
/// Contains the declaration for the DemoMotion class.

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include "MultiplayerPage.h"

/// Class representing the Motion Demo page. This page demonstrates how to set up force 
/// feedback events through the Wwise Sound Engine and Motion Engine.
class DemoMotion : public MultiplayerPage
{
public:

	/// DemoMotion class constructor
	DemoMotion( Menu& in_ParentMenu );

	/// Initializes the demo.
	/// \return True if successful and False otherwise.
	virtual bool Init();

	/// Releases resources used by the demo.
	virtual void Release();

private:

	/// Initializes the controls on the page.
	virtual void InitControls();

	/// Called when a player connects.
	/// Method declared Pure Virtual in MultiplayerPage.h
	virtual void OnPlayerConnect( int in_iPlayerIndex );

	/// Called when a player disconnects.
	/// Method declared Pure Virtual in MultiplayerPage.h
	virtual void OnPlayerDisconnect( int in_iPlayerIndex );

	/// Sets up player motion devices and listeners for all the currently active players.
	bool SetupMotionDevices();

	/// Delegate function for the "Door Slide" buttons.
	void DoorSlide_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// Delegate function for the "Shoot Gun" buttons.
	void GunShot_Pressed( void* in_pSender, ControlEvent* in_pEvent );

	/// The number of controls each player has.
	static const int CONTROLS_PER_PLAYER = 2;

	/// ID for the 'door game object.
	static const AkGameObjectID GAME_OBJECT_DOOR = 100;

	/// IDs for the players as game objects.
	static const AkGameObjectID GAME_OBJECT_PLAYERS[MAX_PLAYERS];
};

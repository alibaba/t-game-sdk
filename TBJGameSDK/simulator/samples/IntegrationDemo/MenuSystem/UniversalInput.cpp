//UniversalInput.cpp
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Defines the methods declared in UniversalInput.h

#include "stdafx.h"

#include <algorithm>
#include "UniversalInput.h"



/////////////////////////////////////////////////////////////////////
// UniversalGamepad Public Functions
/////////////////////////////////////////////////////////////////////

UniversalGamepad::UniversalGamepad( int in_iPlayerIndex, UGDeviceType in_eDeviceType, void* in_pExtraParam )
{
	m_iPlayerIndex   = (AkUInt16) in_iPlayerIndex;
	m_DeviceType     = in_eDeviceType;
	m_pExtraParam    = in_pExtraParam;
	m_PrevBtnState   = 0;
	m_CurBtnState    = 0;
	m_CurRawBtnState = 0;
	m_bConnected     = false;
}

bool UniversalGamepad::IsButtonTriggered( UGButton in_Button ) const
{
	return ( ( m_CurBtnState & in_Button ) > 0 );
}

bool UniversalGamepad::IsButtonDown( UGButton in_Button ) const
{
	return ( ( m_CurRawBtnState & in_Button ) > 0 );
}

bool UniversalGamepad::IsConnected() const
{
	return m_bConnected;
}

void UniversalGamepad::SetState( bool in_bConnected, UGBtnState in_BtnState, UGStickState * in_StickStates )
{
	m_bConnected = in_bConnected;

	// Set the button states...
	m_PrevBtnState = m_CurRawBtnState;
	m_CurRawBtnState = in_BtnState;
	m_CurBtnState = ( m_CurRawBtnState & ~( m_PrevBtnState ) );  // CurBtnState is single trigger!

	memcpy(m_StickStates, in_StickStates, sizeof(m_StickStates));
}

UGBtnState UniversalGamepad::GetCurrentState() const
{
	return m_CurBtnState;
}

UGBtnState UniversalGamepad::GetCurrentRawState() const
{
	return m_CurRawBtnState;
}

AkUInt16 UniversalGamepad::GetPlayerIndex() const
{
	return m_iPlayerIndex;
}

UGDeviceType UniversalGamepad::GetDeviceType() const
{
	return m_DeviceType;
}

void* UniversalGamepad::GetExtraParam() const
{
	return m_pExtraParam;
}

void UniversalGamepad::GetStickPosition( UGThumbstick in_Stick, float &out_x, float &out_y ) const
{	
	out_x = m_StickStates[in_Stick].x;
	out_y = m_StickStates[in_Stick].y;
}

/////////////////////////////////////////////////////////////////////
// UniversalInput Public Functions
/////////////////////////////////////////////////////////////////////

UniversalInput::~UniversalInput()
{
	m_GamePads.clear();
}


bool UniversalInput::AddDevice( AkUInt16 in_nPlayerIndex, UGDeviceType in_eDeviceType, void* in_pExtraParam )
{
	// Check that a device with this playerindex doesn't already exist	
	for ( Iterator it = m_GamePads.begin(); it != m_GamePads.end() && it->GetPlayerIndex() <= in_nPlayerIndex; it++ )
	{
		if ( it->GetPlayerIndex() == in_nPlayerIndex )
		{
			return false;
		}
		
	}
	
	// Add the gamepad to the list
	UniversalGamepad sNewGamepad( in_nPlayerIndex, in_eDeviceType, in_pExtraParam );
	m_GamePads.push_back( sNewGamepad );
	std::sort( m_GamePads.begin(), m_GamePads.end(), CompareGamepads );

	return true;
}


const UniversalGamepad* UniversalInput::GetGamepad( AkUInt16 in_nPlayerIndex ) const
{
	std::vector<UniversalGamepad>::const_iterator it;
	for ( it = m_GamePads.begin(); it != m_GamePads.end() && it->GetPlayerIndex() <= in_nPlayerIndex; it++ )
	{
		if ( it->GetPlayerIndex() == in_nPlayerIndex )
		{
			return &(*it);
		}
	}
	return NULL;
}


UniversalInput::Iterator UniversalInput::Begin()
{
	return m_GamePads.begin();
}


UniversalInput::Iterator UniversalInput::End()
{
	return m_GamePads.end();
}


AkUInt16 UniversalInput::NumDevices() const
{
	return (AkUInt16) m_GamePads.size();
}


AkUInt16 UniversalInput::NumDevices( UGDeviceType in_eDeviceType ) const
{
	std::vector<UniversalGamepad>::const_iterator it;
	AkUInt16 count = 0;

	for ( it = m_GamePads.begin(); it != m_GamePads.end(); it++ )
	{
		if ( it->GetDeviceType() == in_eDeviceType )
		{
			++count;
		}
	}

	return count;
}


bool UniversalInput::IsPlayerConnected( AkUInt16 in_nPlayerIndex ) const
{
	std::vector<UniversalGamepad>::const_iterator it;
	for ( it = m_GamePads.begin(); it != m_GamePads.end() && it->GetPlayerIndex() <= in_nPlayerIndex; it++ )
	{
		if ( it->GetPlayerIndex() == in_nPlayerIndex )
		{
			return it->IsConnected();
		}
	}
	return false;
}


bool UniversalInput::RemoveDevice( AkUInt16 in_nPlayerIndex )
{
	for ( Iterator it = m_GamePads.begin(); it != m_GamePads.end() && it->GetPlayerIndex() <= in_nPlayerIndex; it++ )
	{
		if ( it->GetPlayerIndex() == in_nPlayerIndex )
		{
			m_GamePads.erase( it );
			return true;
		}
	}

	return false;
}


bool UniversalInput::SetState( AkUInt16 in_nPlayerIndex, bool in_bConnected, UGBtnState in_BtnState, UGStickState *in_StickStates )
{
	for ( Iterator it = m_GamePads.begin(); it != m_GamePads.end() && it->GetPlayerIndex() <= in_nPlayerIndex; it++ )
	{
		if ( it->GetPlayerIndex() == in_nPlayerIndex )
		{
			it->SetState( in_bConnected, in_BtnState, in_StickStates );
			return true;
		}
	}
	return false;
}



/////////////////////////////////////////////////////////////////////
// UniversalInput Private Functions
/////////////////////////////////////////////////////////////////////

bool UniversalInput::CompareGamepads(
	UniversalGamepad in_sFirst, 
	UniversalGamepad in_sSecond 
	)
{
	return ( in_sFirst.GetPlayerIndex() < in_sSecond.GetPlayerIndex() );
}

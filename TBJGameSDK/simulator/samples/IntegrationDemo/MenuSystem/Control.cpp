// Control.cpp
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Defines the methods declared in Control.h


#include "stdafx.h"

#include "Control.h"


/////////////////////////////////////////////////////////////////////
// Control Public Methods
/////////////////////////////////////////////////////////////////////

Control::Control( Page& in_pParentPage )
{
	m_pParentPage = &in_pParentPage;
	m_szLabel = "";
	m_pDelegateFunc = NULL;	
	m_iXPos = 0;
	m_iYPos = 0;
}

void Control::SetDelegate( PageMFP in_pDelegateFunction )
{
	m_pDelegateFunc = in_pDelegateFunction;
}

void Control::CallDelegate( ControlEvent* in_pEvent ) const
{
	if ( m_pParentPage && m_pDelegateFunc )
	{
		(m_pParentPage->*m_pDelegateFunc)( (void*)this, in_pEvent );
	}
}

void Control::SetLabel( const char* in_szLabel )
{
	m_szLabel = in_szLabel;
}

void Control::SetPosition( int in_iXPos, int in_iYPos )
{
	m_iXPos = in_iXPos;
	m_iYPos = in_iYPos;
}

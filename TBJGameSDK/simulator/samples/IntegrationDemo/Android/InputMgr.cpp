// InputMgr.h
// Copyright (C) 2011 Audiokinetic Inc.
/// \file 
/// Defines the methods declared in InputMgr.h

#include "stdafx.h"

#include "InputMgr.h"
#include "Platform.h"
#include "UniversalInput.h"

extern void DisplayKeyboard(android_app* app, bool pShow);

int32_t InputMgr::HandleInput(struct android_app* app, AInputEvent* event)
{
	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_KEY)
	{
		DisplayKeyboard( app, true );
	    return 0;
	}

	if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
	{
		UGStickState stickState[] = {{0.f, 0.f},{0.f,0.f} };
		UGBtnState btnState = 0;

		int32_t iKeyCode = AKeyEvent_getKeyCode(event);
		switch ( iKeyCode )
		{
		case AKEYCODE_W:
		case AKEYCODE_DPAD_UP:
			btnState |=	UG_DPAD_UP;
			break;
		case AKEYCODE_S:
		case AKEYCODE_DPAD_DOWN:
			btnState |=	UG_DPAD_DOWN;
			break;
		case AKEYCODE_A:
		case AKEYCODE_DPAD_LEFT:
			btnState |=	UG_DPAD_LEFT;
			break;
		case AKEYCODE_D:
		case AKEYCODE_DPAD_RIGHT:
			btnState |=	UG_DPAD_RIGHT;
			break;
		case AKEYCODE_ENTER:
		case AKEYCODE_DPAD_CENTER:
			btnState |=	UG_BUTTON1;
			break;
		case AKEYCODE_SPACE:
		case AKEYCODE_BACK:
			btnState |=	UG_BUTTON2;
			break;
		case AKEYCODE_Z:
			btnState |=	UG_BUTTON3;
			break;
		case AKEYCODE_X:
			btnState |=	UG_BUTTON4;
			break;
		case AKEYCODE_C:
			btnState |=	UG_BUTTON5;
			break;
		case AKEYCODE_V:
			btnState |=	UG_BUTTON6;
			break;
		case AKEYCODE_B:
			btnState |=	UG_BUTTON7;
			break;
		case AKEYCODE_N:
			btnState |=	UG_BUTTON8;
			break;
		case AKEYCODE_MENU: // make sure soft keyboard is usable on galaxy nexus
			DisplayKeyboard( app, true );
			break;
		}

		if( btnState )
		{
			InputMgr* THIS = (InputMgr*)app->userData;
			THIS->UniversalInputAdapter()->SetState(1, true, btnState, stickState);
		}
	}

	return 0;
}


UGStickState	g_sticksState[2];

InputMgr::~InputMgr()
{
}

bool InputMgr::Init(
	void* ,
	AkOSChar* ,
	unsigned int 
)
{
	m_pUInput = new UniversalInput;
	m_pUInput->AddDevice( 1 , UGDeviceType_GAMEPAD );
	memset(g_sticksState, 0, sizeof(g_sticksState));
	
	return true;
}

UniversalInput* InputMgr::UniversalInputAdapter() const
{
	return m_pUInput;
}

void InputMgr::Release()
{
	if ( m_pUInput )
	{
		delete m_pUInput;
		m_pUInput = NULL;
	}
}

void InputMgr::Update()
{
}

void InputMgr::Flush()
{
	UGStickState stickState[] = {{0.f, 0.f},{0.f,0.f} };
	UGBtnState btnState = 0;
	m_pUInput->SetState(1, true, btnState, stickState);
}



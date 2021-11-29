// Menu.h
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Contains the declaration of the Menu class.

#pragma once

#include <stack>
#include "Page.h"
#include "UniversalInput.h"


/// The Menu class is the main object for the Menu system. It manages all the updating
/// and drawing of all Pages.
class Menu
{
public:

	/// Menu class constructor.
	Menu();

	/// Menu class destructor.
	~Menu();

	/// Initializes the Menu system.
	/// \warning This method must be called prior to using the Menu.
	void Init( 
		UniversalInput* in_pInput,	///< - Pointer to a UniversalInput object to use as input
		int in_iWidth,				///< - The width of the screen
		int in_iHeight,				///< - The height of the screen
		int in_iFrameRate,			///< - The application's framerate
		Page& in_BasePage			///< - The base Page object to use as the main menu page
		);

	/// Ends the Menu system and releases all used resources.
	void Release();

	/// Updates the Menu system.
	/// \return False if the user wants to quit the application and True otherwise.
	/// \note Call this method once per frame, prior to drawing.
	bool Update();
	
	void OnPointerEvent( PointerEventType in_eType, int in_x, int in_y );

	/// Draws the Menu to the screen.
	/// \note Call this method once per frame, after calling Update.
	void Draw();

	/// Signals that the user wants to quit the application.
	void QuitApplication();

	/// Acceessor method to the UniversalInput object being used by the Menu system.
	/// \return Pointer to the UniversalInput object.
	UniversalInput* Input() const;
	
	/// Accessor method to the framerate that the menu has been set to.
	/// \return The framerate the system is set to.
	int GetFrameRate() const;

	/// Accessor method to the height of the menu screen.
	/// \return The height of the menu screen.
	int GetHeight() const;

	/// Accessor method to the width of the menu screen.
	/// \return The width of the menu screen.
	int GetWidth() const;

	/// Accessor method to the number of ticks (ie- calls to update) since the Menu started.
	/// \return The number of ticks since the Menu started.
	int GetTickCount() const;

	/// Adds a new Page to the top of the stack of currently active pages.
	void StackPage( 
		Page* in_pPage		///< - Pointer to the Page being added onto the stack
		);

	/// Removes and deletes the Page at the top of the stack of currently active pages.
	void PopPageStack();

	void Back();

private:

	/// Height of the menu screen.
	int m_iMenuHeight;

	/// Width of the menu screen.
	int m_iMenuWidth;

	/// Number of ticks (ie- calls to update) since the Menu was started.
	int m_iTickCount;

	/// The framerate (frames per second) that the Menu system has been set to run on.
	int m_iFrameRate;

	/// Set to true when the user wants to quit the application.
	bool m_bQuitApp;

	/// Pointer to the Universal Input object being used by the Menu system.
	UniversalInput* m_pInput;

	/// The container for the open menu Pages.
	std::stack<Page*> m_PageStack;
};

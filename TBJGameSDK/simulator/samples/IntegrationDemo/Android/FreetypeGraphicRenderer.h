// FreetypeGraphicRenderer.h
// Copyright (C) 2011 Audiokinetic Inc.
/// \file
/// Draw strings to a RGB 565 buffer

#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <string>

#ifndef NDK_APP
#include "Drawing.h"
#endif

#include "stb_truetype.h"


using std::string;

/// Replaces tags by their platform-specific values
void ReplaceTags( string& io_strTagString );

class FreetypeGraphicRenderer
{
public:
	FreetypeGraphicRenderer():
	m_width(0),
	m_height(0),
	m_pWindowBuffer(NULL)
	{}


	~FreetypeGraphicRenderer(){};

	/// Initializes the system's drawing engine for usage.
	/// \return True if the system has been initialized, false otherwise.
	/// \sa TermDrawing()
	virtual bool InitDrawing(
							void* in_pParam,						///< - Pointer to any platform specific parameter (eg. the window handle in Windows)
							AkOSChar* in_szErrorBuffer,				///< - Buffer where error details will be written (if the function returns false)
							unsigned int in_unErrorBufferCharCount,	///< - Number of characters available in in_szErrorBuffer, including terminating NULL character
							int	in_windowWidth = 640,				///< - window widht in pixels
							int in_windowHeight = 480				///< - window height in pixels
							);


	/// Begins a drawing sequence
	/// \sa DoneDrawing()
	virtual void BeginDrawing();


	/// Draws a string of text on the screen at a given point.
	/// \note The coordinates (0, 0) refer to the top-left corner of the screen.
	/// \warning This function must be called between calls to BeginDrawing and DoneDrawing.
	/// \sa BeginDrawing(), DoneDrawing()
	virtual void DrawTextOnScreen(
						  const char* in_szText,	///< - The string to draw on the screen
						  int in_iXPos,			    ///< - The X value of the drawing position
						  int in_iYPos,			    ///< - The Y value of the drawing position
						  DrawStyle in_eDrawStyle = DrawStyle_Control		///< - The style with which to draw the text
						  );

	/// Ends the drawing sequence and outputs the drawing.
	/// \sa BeginDrawing()
	virtual void DoneDrawing();


	/// Closes the drawing engine and releases any used resources.
	/// \sa InitDrawing()
	virtual void TermDrawing();


	/// Gets the height of a line of text drawn using the given style.
	/// \return The height of a line of text, in pixels.
	virtual int GetLineHeight(
							  DrawStyle in_eDrawStyle		///< - The style of the text being queried
							  );

	/// Gets the width of the window in pixels.
	/// \return The width of the window in pixels.
	virtual int GetWindowWidth() { return m_width; }

	/// Gets the height of the window in pixels.
	/// \return The height of the window in pixels.
	virtual int GetWindowHeight() { return m_height; }

	
	/// Access the data buffer
	/// \return the pointers to bits in the buffer
	const uint16_t* GetWindowBuffer();

private:

	bool InitFreetype();
	void printString( const char* pStr, int in_x, int in_y, DrawStyle in_eDrawStyle);
	void draw_bitmap( unsigned char* bitmap, int x, int y, int w, int h, uint32_t color);
	
	unsigned int m_width;
	unsigned int m_height;
	
	stbtt_fontinfo m_font;
	stbtt_fontinfo m_fontBold;
	
	uint16_t*	  m_pWindowBuffer;

	unsigned char *m_pFontBuffer;
	unsigned char *m_pFontBoldBuffer;
};

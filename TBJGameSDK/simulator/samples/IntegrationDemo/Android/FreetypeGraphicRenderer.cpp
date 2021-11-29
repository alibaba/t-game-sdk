// FreetypeGraphicRenderer.cpp
// Copyright (C) 2011 Audiokinetic Inc
/// \file 
/// Print text in a RGB656 buffer
/// Freetype errors are logged but there is no fallback code to handle errors.

/////////////////////////
//  INCLUDES
/////////////////////////

#include "stdafx.h"
#include "Platform.h"
#include "FreetypeGraphicRenderer.h"

#include <string>
#include <android/log.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "IntegrationDemo", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "IntegrationDemo", __VA_ARGS__))

using std::string;

#define DEFAULT_DRAW_COLOUR		0xFFFFFFFF	// White
#define SELECTED_DRAW_COLOUR	0xFFFFFF00  // Yellow 0xFBE0/
#define TITLE_DRAW_COLOUR   	0xFFFFFFFF	// White  0xFFFF
#define ERROR_DRAW_COLOUR	 	0xFFFF0000  // Red	0xf800

/////////////////////////
//  FUNCTIONS
/////////////////////////

const uint16_t* FreetypeGraphicRenderer::GetWindowBuffer()
{
	return m_pWindowBuffer;
}

void FreetypeGraphicRenderer::BeginDrawing()
{
	// Wipe out the buffer
	memset(m_pWindowBuffer,0,m_width*m_height*sizeof(uint16_t));
}

void FreetypeGraphicRenderer::DoneDrawing()
{
	// Nothing to do
}

void FreetypeGraphicRenderer::printString( const char* pStr, int in_x, int in_y, DrawStyle in_eDrawStyle)
{
	// Set colour based on the desired DrawStyle
	uint32_t color;
	stbtt_fontinfo* pFace = &m_font;
	
	switch ( in_eDrawStyle )
	{
	case DrawStyle_Title:
		color = TITLE_DRAW_COLOUR;
		pFace = &m_fontBold;
		break;
	case DrawStyle_Selected:
		color = SELECTED_DRAW_COLOUR;
		pFace = &m_fontBold;
		break;
	case DrawStyle_Error:
		color = ERROR_DRAW_COLOUR;
		break;
	case DrawStyle_Text:
		color = DEFAULT_DRAW_COLOUR;
		break;
	case DrawStyle_Control:
	default:
		color = DEFAULT_DRAW_COLOUR;
		break;
	}

	int b_w = 32; /* bitmap width */
    int b_h = 32; /* bitmap height */    

	int ascent, descent, lineGap;
	float scale = stbtt_ScaleForPixelHeight(pFace, GetLineHeight(in_eDrawStyle));
	stbtt_GetFontVMetrics(pFace, &ascent, &descent, &lineGap);

	ascent *= scale;
	descent *= scale;

	/* create a bitmap for the phrase */
	unsigned char* bitmap = (unsigned char*)malloc(b_w * b_h);

	int i;
	int x = 0;
	int height = 0;
	for (i = 0; i < strlen(pStr); ++i)
	{
		memset(bitmap, 0, b_w * b_h);

		/* get bounding box for character (may be offset to account for chars that dip above or below the line */
		int c_x1, c_y1, c_x2, c_y2;
		stbtt_GetCodepointBitmapBox(pFace, pStr[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

		/* compute y (different characters have different heights */
		int y = ascent + c_y1;

		/* render character (stride and offset is important here) */	
		stbtt_MakeCodepointBitmap(pFace, bitmap, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, pStr[i]);

		if (c_y2 - c_y1 > height)
			height = c_y2 - c_y1;

		/* how wide is this character */
		int ax;
		stbtt_GetCodepointHMetrics(pFace, pStr[i], &ax, 0);		

		// now, draw to our target surface
		draw_bitmap( bitmap, in_x+x, in_y+y, b_w, height, color );

		x += ax * scale + 2/*kerning doesn't work, so 2 pixel between each*/;
	}
	free(bitmap);

}

void FreetypeGraphicRenderer::draw_bitmap( unsigned char* bitmap, int x, int y, int w, int h, uint32_t color)
{
	int  i, j, p, q;

	int  x_max = x + w;
	int  y_max = y + h;

	float fR = float((color & 0XFF0000) >> 16) / 255.f;
	float fG = float((color & 0XFF00) >> 8) /255.f ;
	float fB = float(color & 0XFF) / 255.f;

	for ( j = y, q = 0; j < y_max; j++, q++ )
	{
		// Outside range
		if( j > m_height || j < 0 )
			continue;

		int jOffset = j*m_width;
		int qOffset = q * w;
		for ( i = x, p = 0; i < x_max; i++, p++ )
		{
			// Outside range
			if ( i >= m_width  || i<0 )
				continue;

			// Converting to RGB 565
			uint16_t R,G,B;
			float greyLevel = bitmap[qOffset+p] / 255.f;
			R = greyLevel * fR * 0X1F;
			R <<= 11;
			G = greyLevel * fG * 0X1F;
			G <<= 6;
			B = greyLevel * fB * 0X1F;
		
			// Write the value to the buffer
			m_pWindowBuffer[jOffset+i] += R+B+G;
		}
	}
}

bool FreetypeGraphicRenderer::InitFreetype()
{
	const char *szFile = "/system/fonts/DroidSans.ttf";
	int file=0;
	if((file=open(szFile,O_RDONLY)) < -1)
	{
		LOGI("Could not open /system/fonts/DroidSans.ttf");
		return false;
	}
	
	struct stat fileStat;
	fstat(file,&fileStat);

	m_pFontBuffer = (unsigned char*)malloc(fileStat.st_size);
		
	read(file, m_pFontBuffer, fileStat.st_size);
	close(file);

	int retFont = stbtt_InitFont(&m_font, m_pFontBuffer, 0);	

	szFile = "/system/fonts/DroidSansBold.ttf";
	if((file=open(szFile,O_RDONLY)) < -1)
	{
		LOGI("Could not open /system/fonts/DroidSansBold.ttf");
		return false;
	}
	
	fstat(file,&fileStat);

	m_pFontBoldBuffer = (unsigned char*)malloc(fileStat.st_size);
	read(file ,m_pFontBoldBuffer, fileStat.st_size);
	close(file);

	int retBold = stbtt_InitFont(&m_fontBold, m_pFontBoldBuffer, 0);	

	if (!retFont)
	{
		TermDrawing();
		LOGI("Error loading fonts.");
		return false;
	}

	if (!retBold)
	{
		free(m_pFontBoldBuffer);
		m_pFontBoldBuffer = NULL;
		memcpy(&m_fontBold, &m_font, sizeof(m_font));	//Use the same fonts.
	}

	return true;
}

bool FreetypeGraphicRenderer::InitDrawing( void* in_pParam, AkOSChar* in_szErrorBuffer, unsigned int in_unErrorBufferCharCount, int in_windowWidth, int in_windowHeight )
{
	m_width = in_windowWidth;
	m_height = in_windowHeight;

	m_pWindowBuffer = (AkUInt16*) malloc(m_width*m_height*sizeof(uint16_t));

	return m_pWindowBuffer && InitFreetype();
}


void FreetypeGraphicRenderer::DrawTextOnScreen( const char* in_szText, int in_iXPos, int in_iYPos, DrawStyle in_eDrawStyle )
{
	string translated = in_szText;
	ReplaceTags( translated );

	char tmpStr[INTEGRATIONDEMO_MAX_MESSAGE];
	char* token;

	AKPLATFORM::SafeStrCpy(tmpStr, translated.c_str(), INTEGRATIONDEMO_MAX_MESSAGE);

	token = strtok( tmpStr, "\n" );
	while ( token )
	{
		printString( token , in_iXPos, in_iYPos, in_eDrawStyle );
		in_iYPos += GetLineHeight( in_eDrawStyle );
		token = strtok( NULL, "\n" );
	}
}

void FreetypeGraphicRenderer::TermDrawing()
{
	if (m_pFontBuffer)
		free(m_pFontBuffer);

	if (m_pFontBoldBuffer)
		free(m_pFontBoldBuffer);

	if( m_pWindowBuffer )
	{
		free(m_pWindowBuffer);
		m_pWindowBuffer = NULL;
	}
}

int FreetypeGraphicRenderer::GetLineHeight( DrawStyle in_eDrawStyle )
{
	if ( in_eDrawStyle == DrawStyle_Title )
	{
		return 26;  // Change this if the font size is changed in InitDrawing()!!
	}
	else if ( in_eDrawStyle == DrawStyle_Text )
	{
		return 18;	// Change this if the font size is changed in InitDrawing()!!
	}
	else
	{
		return 20;	// Change this if the font size is changed in InitDrawing()!!
	}
}

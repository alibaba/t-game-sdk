// Helpers.cpp
// Copyright (C) 2010 Audiokinetic Inc
/// \file 
/// Implements all functions declared in Helpers.h

#include "stdafx.h"
#include "Helpers.h"
#include "Drawing.h"

void IntegrationDemoHelpers::WordWrapString( std::string& io_strWrapped, const size_t in_nMaxWidth )
{
	if ( in_nMaxWidth < 2 )
		return;

	size_t nLineStartIndex = 0;
	size_t nCurrentIndex = 0;
	size_t nNewLine = (size_t)-1;
	size_t nStringEndIndex = io_strWrapped.length();

	bool bFoundNonSpaceCharacter = false;
	std::string spacesAtBeginningOfLine;

	while ( nLineStartIndex < nStringEndIndex )
	{
		while ( ( nCurrentIndex < nStringEndIndex ) && ( ( nCurrentIndex - nLineStartIndex ) < in_nMaxWidth ) )
		{
			const char thisChar = io_strWrapped[nCurrentIndex];

			if ( thisChar == L'\n' )
			{
				// We've reached the end of a hardcoded newline. Let's
				// start again at the next line.
				nCurrentIndex++;
				nLineStartIndex = nCurrentIndex;
				spacesAtBeginningOfLine.clear();
				bFoundNonSpaceCharacter = false;
			}
			else if ( thisChar == L' ' )
			{
				if ( ! bFoundNonSpaceCharacter )
				{
					spacesAtBeginningOfLine.append( io_strWrapped.substr( nCurrentIndex, 1 ) );
				}

				if ( ( (nCurrentIndex+1) < nStringEndIndex )
					&& ( io_strWrapped[nCurrentIndex+1] != L' ' ) )
				{
					// This is a potential location for a newline
					nNewLine = nCurrentIndex;
				}

				nCurrentIndex++;
			}
			else
			{
				if ( ! bFoundNonSpaceCharacter )
				{
					if ( thisChar == L'*' || thisChar == L'-' )
					{
						spacesAtBeginningOfLine.append( " " );
					}
					else
					{
						bFoundNonSpaceCharacter = true;
					}
				}

				nCurrentIndex++;
			}
		}

		if ( nNewLine == (size_t)-1 )
		{
			// Couldn't find a suitable place to cut, let's quit
			return;
		}

		if ( nCurrentIndex >= nStringEndIndex )
		{
			// We reached the end of the string, we're done
			return;
		}

		io_strWrapped[nNewLine] = L'\n'; // -----8<--- Cut here ---------

		// Move to start of next line
		nLineStartIndex = nCurrentIndex = nNewLine+1; 

		if ( ( spacesAtBeginningOfLine.length() > 0 ) && ( nLineStartIndex < nStringEndIndex ) )
		{
			io_strWrapped.insert( nLineStartIndex, spacesAtBeginningOfLine );
			nStringEndIndex += spacesAtBeginningOfLine.length();
		}
		
		spacesAtBeginningOfLine.clear();
		bFoundNonSpaceCharacter = false;
		nNewLine = (size_t)-1;
	}
}


std::string IntegrationDemoHelpers::WordWrapString( const char* in_szOriginal, const size_t in_nMaxWidth )
{
	std::string strText( in_szOriginal );
	WordWrapString( strText, in_nMaxWidth );
	return strText;
}

/// Replaces tags by their platform-specific values
void ReplaceTags( string& io_strTagString )
{
	static const char* TAGS[][2] = {
		{ "<<UG_BUTTON1>>",  UG_BUTTON1_NAME  },
		{ "<<UG_BUTTON2>>",  UG_BUTTON2_NAME  },
		{ "<<UG_BUTTON3>>",  UG_BUTTON3_NAME  },
		{ "<<UG_BUTTON4>>",  UG_BUTTON4_NAME  },
		{ "<<UG_BUTTON5>>",  UG_BUTTON5_NAME  },
		{ "<<UG_BUTTON6>>",  UG_BUTTON6_NAME  },
		{ "<<UG_BUTTON7>>",  UG_BUTTON7_NAME  },
		{ "<<UG_BUTTON8>>",  UG_BUTTON8_NAME  },
		{ "<<UG_BUTTON9>>",  UG_BUTTON9_NAME  },
		{ "<<UG_BUTTON10>>", UG_BUTTON10_NAME },
		{ "<<UG_BUTTON11>>", UG_BUTTON11_NAME },
		{ "<<OS_BUTTON>>",   OS_BUTTON_NAME   },
		{ "<<HELP_BUTTON>>",   HELP_BUTTON    },
		{ "<<ACTION_BUTTON>>", ACTION_BUTTON  },
		{ "<<BACK_BUTTON>>",   BACK_BUTTON    },
		{ "<<DIRECTIONAL_TYPE>>", DIRECTIONAL_TYPE },
		{ "<<UG_RIGHT_STICK>>", UG_RIGHT_STICK },
		{ "<<UG_LEFT_STICK>>", UG_LEFT_STICK },
		{ "", "" } // This must be the last one!
	};

	// Cycle through the available tags
	for ( int i = 0; strlen( TAGS[i][0] ) > 0; i++ )
	{
		// Do a find and replace for the current tag
		size_t j = io_strTagString.find( TAGS[i][0] );
		if ( j != string::npos )
		{
			io_strTagString.replace( j, strlen( TAGS[i][0] ), TAGS[i][1] );
			--i; //Repeat to find all instance of this tag in the string
		}
	}
}

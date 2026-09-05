// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	URL.cpp: Various file-management functions.
=============================================================================*/

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CString.h"

bool FURL::HasOption( const TCHAR* Test ) const
{
	return GetOption( Test, NULL ) != NULL;
}

const TCHAR* FURL::GetOption( const TCHAR* Match, const TCHAR* Default ) const
{
	const int32 Len = FCString::Strlen(Match);
	
	if( Len > 0 )
	{
		for( int32 i = 0; i < Op.Num(); i++ ) 
		{
			const TCHAR* s = *Op[i];
			if( FCString::Strnicmp( s, Match, Len ) == 0 ) 
			{
				if (s[Len-1] == '=' || s[Len] == '=' || s[Len] == '\0') 
				{
					return s + Len;
				}
			}
		}
	}

	return Default;
}

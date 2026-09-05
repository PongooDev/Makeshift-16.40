// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/Parse.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CString.h"
#include "Engine/Source/Runtime/Core/Public/Misc/Char.h"

//
// Checks if a command-line parameter exists in the stream.
//
bool FParse::Param( const TCHAR* Stream, const TCHAR* Param )
{
	const TCHAR* Start = Stream;
	if( *Stream )
	{
		while( (Start=FCString::Strifind(Start,Param,true)) != NULL )
		{
			if( Start>Stream && (Start[-1]=='-' || Start[-1]=='/') && 
				(Stream > (Start - 2) || FChar::IsWhitespace(Start[-2]))) // Reject if the character before '-' or '/' is not a whitespace
			{
				const TCHAR* End = Start + FCString::Strlen(Param);
				if ( End == NULL || *End == 0 || FChar::IsWhitespace(*End) )
				{
					return true;
				}
			}

			Start++;
		}
	}
	return false;
}

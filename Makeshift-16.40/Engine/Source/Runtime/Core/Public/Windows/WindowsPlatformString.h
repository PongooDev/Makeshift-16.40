// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include <tchar.h>

#include <cwchar>
#include <cstdio>
#include <cstring>
#include <stdarg.h>

#pragma warning(push)
#pragma warning(disable: 4996)

/**
 * Windows string implementation.
 */
struct FWindowsPlatformString
{
	/**
	 * Wide character implementation
	 **/
	static FORCEINLINE WIDECHAR* Strcpy(WIDECHAR* Dest, SIZE_T DestCount, const WIDECHAR* Src)
	{
		return (WIDECHAR*)wcscpy(Dest, Src);
	}

	static FORCEINLINE WIDECHAR* Strncpy(WIDECHAR* Dest, const WIDECHAR* Src, SIZE_T MaxLen)
	{
		wcsncpy(Dest, Src, MaxLen-1);
		Dest[MaxLen-1] = 0;
		return Dest;
	}

	static FORCEINLINE WIDECHAR* Strcat(WIDECHAR* Dest, SIZE_T DestCount, const WIDECHAR* Src)
	{
		return (WIDECHAR*)wcscat(Dest, Src);
	}

	static FORCEINLINE int32 Strcmp( const WIDECHAR* String1, const WIDECHAR* String2 )
	{
		return wcscmp(String1, String2);
	}

	static FORCEINLINE int32 Strncmp( const WIDECHAR* String1, const WIDECHAR* String2, SIZE_T Count )
	{
		return wcsncmp( String1, String2, Count );
	}

	static FORCEINLINE int32 Stricmp( const WIDECHAR* String1, const WIDECHAR* String2 )
	{
		return _wcsicmp( String1, String2 );
	}

	static FORCEINLINE int32 Strnicmp( const WIDECHAR* String1, const WIDECHAR* String2, SIZE_T Count )
	{
		return _wcsnicmp( String1, String2, Count );
	}

	static FORCEINLINE const WIDECHAR* Strstr( const WIDECHAR* String, const WIDECHAR* Find)
	{
		return _tcsstr( String, Find );
	}

	static FORCEINLINE int32 Strlen( const WIDECHAR* String )
	{
		return (int32)wcslen( String );
	}

	static FORCEINLINE int32 GetVarArgs( WIDECHAR* Dest, SIZE_T DestSize, const WIDECHAR*& Fmt, va_list ArgPtr )
	{
		int32 Result = vswprintf(Dest, DestSize, Fmt, ArgPtr);
		va_end( ArgPtr );
		return Result;
	}

	/**
	 * Ansi implementation
	 **/
	static FORCEINLINE ANSICHAR* Strcpy(ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src)
	{
		return (ANSICHAR*)strcpy(Dest, Src);
	}

	static FORCEINLINE ANSICHAR* Strncpy(ANSICHAR* Dest, const ANSICHAR* Src, SIZE_T MaxLen)
	{
		::strncpy(Dest, Src, MaxLen);
		Dest[MaxLen-1]=0;
		return Dest;
	}

	static FORCEINLINE ANSICHAR* Strcat(ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src)
	{
		return (ANSICHAR*)strcat( Dest, Src );
	}

	static FORCEINLINE int32 Strcmp( const ANSICHAR* String1, const ANSICHAR* String2 )
	{
		return strcmp(String1, String2);
	}

	static FORCEINLINE int32 Strncmp( const ANSICHAR* String1, const ANSICHAR* String2, SIZE_T Count )
	{
		return strncmp( String1, String2, Count );
	}

	static FORCEINLINE int32 Stricmp( const ANSICHAR* String1, const ANSICHAR* String2 )
	{
		return _stricmp( String1, String2 );
	}

	static FORCEINLINE int32 Strnicmp( const ANSICHAR* String1, const ANSICHAR* String2, SIZE_T Count )
	{
		return _strnicmp( String1, String2, Count );
	}

	static FORCEINLINE const ANSICHAR* Strstr( const ANSICHAR* String, const ANSICHAR* Find)
	{
		return strstr(String, Find);
	}

	static FORCEINLINE int32 Strlen( const ANSICHAR* String )
	{
		return (int32)strlen( String );
	}

	static FORCEINLINE int32 GetVarArgs( ANSICHAR* Dest, SIZE_T DestSize, const ANSICHAR*& Fmt, va_list ArgPtr )
	{
		int32 Result = vsnprintf( Dest, DestSize, Fmt, ArgPtr );
		va_end( ArgPtr );
		return (Result != -1 && Result < (int32)DestSize) ? Result : -1;
	}
};

#pragma warning(pop) // 'function' was was declared deprecated  (needed for the secure string functions)

typedef FWindowsPlatformString FPlatformString;

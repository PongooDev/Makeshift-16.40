// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/VarArgs.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"
#include "Engine/Source/Runtime/Core/Public/Windows/WindowsPlatformString.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsArrayOrRefOfType.h"
#include "Engine/Source/Runtime/Core/Public/Templates/IsValidVariadicFunctionArg.h"
#include "Engine/Source/Runtime/Core/Public/Templates/AndOrNot.h"

#define MAX_SPRINTF 1024

/**
 *	Set of basic string utility functions operating on plain C strings. In addition to the
 *	wrapped C string API,this struct also contains a set of widely used utility functions that
 *	operate on c strings.
 *	There is a specialized implementation for ANSICHAR and WIDECHAR strings provided. To access these
 *	functionality, the convenience typedefs FCString and FCStringAnsi are provided.
 **/
template <typename T>
struct TCString
{
	typedef T CharType;

	/**
	 * strcpy wrapper
	 *
	 * @param Dest - destination string to copy to
	 * @param Destcount - size of Dest in characters
	 * @param Src - source string
	 * @return destination string
	 */
	static FORCEINLINE CharType* Strcpy( CharType* Dest, SIZE_T DestCount, const CharType* Src );

	/**
	 * Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed.
	 *
	 * @param Dest - destination buffer to copy to
	 * @param Src - source buffer to copy from
	 * @param MaxLen - max length of the buffer (including null-terminator)
	 * @return pointer to resulting string buffer
	 */
	static FORCEINLINE CharType* Strncpy( CharType* Dest, const CharType* Src, int32 MaxLen );

	/**
	 * strcpy wrapper
	 * (templated version to automatically handle static destination array case)
	 *
	 * @param Dest - destination string to copy to
	 * @param Src - source string
	 * @return destination string
	 */
	template<SIZE_T DestCount>
	static FORCEINLINE CharType* Strcpy( CharType (&Dest)[DestCount], const CharType* Src )
	{
		return Strcpy( Dest, DestCount, Src );
	}

	/**
	 * strcat wrapper
	 *
	 * @param Dest - destination buffer to append to
	 * @param Src - source buffer to copy from
	 * @return pointer to resulting string buffer
	 */
	static FORCEINLINE CharType* Strcat( CharType* Dest, SIZE_T DestCount, const CharType* Src );

	/**
	 * strcat wrapper
	 * (templated version to automatically handle static destination array case)
	 *
	 * @param Dest - destination string to copy to
	 * @param Src - source string
	 * @return destination string
	 */
	template<SIZE_T DestCount>
	static FORCEINLINE CharType* Strcat( CharType (&Dest)[DestCount], const CharType* Src )
	{
		return Strcat( Dest, DestCount, Src );
	}

	/**
	 * Concatenate a string with length checking.
	 *
	 * @param Dest - destination buffer to append to
	 * @param Src - source buffer to copy from
	 * @param MaxLen - max length of the buffer
	 * @return pointer to resulting string buffer
	 */
	static inline CharType* Strncat( CharType* Dest, const CharType* Src, int32 MaxLen )
	{
		int32 Len = Strlen(Dest);
		CharType* NewDest = Dest + Len;
		if( (MaxLen-=Len) > 0 )
		{
			Strncpy( NewDest, Src, MaxLen );
		}
		return Dest;
	}

	/**
	 * strcmp wrapper
	 **/
	static FORCEINLINE int32 Strcmp( const CharType* String1, const CharType* String2 );

	/**
	 * strncmp wrapper
	 */
	static FORCEINLINE int32 Strncmp( const CharType* String1, const CharType* String2, SIZE_T Count);

	/**
	 * stricmp wrapper
	 */
	static FORCEINLINE int32 Stricmp( const CharType* String1, const CharType* String2 );

	/**
	 * strnicmp wrapper
	 */
	static FORCEINLINE int32 Strnicmp( const CharType* String1, const CharType* String2, SIZE_T Count );

	/**
	 * strlen wrapper
	 */
	static FORCEINLINE int32 Strlen( const CharType* String );

	/**
	* Standard string formatted print.
	* @warning: make sure code using FCString::Sprintf allocates enough (>= MAX_SPRINTF) memory for the destination buffer
	*/
	template <typename FmtType, typename... Types>
	static int32 Sprintf(CharType* Dest, const FmtType& Fmt, Types... Args)
	{
		static_assert(TIsArrayOrRefOfType<FmtType, CharType>::Value, "Formatting string must be a literal string of the same character type as template.");
		static_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to TCString::Sprintf");

		return SprintfImpl(Dest, Fmt, Args...);
	}

	/**
	 * Safe string formatted print.
	 */
	template <typename FmtType, typename... Types>
	static int32 Snprintf(CharType* Dest, int32 DestSize, const FmtType& Fmt, Types... Args)
	{
		static_assert(TIsArrayOrRefOfType<FmtType, CharType>::Value, "Formatting string must be a literal string of the same character type as template.");
		static_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to TCString::Snprintf");

		return SnprintfImpl(Dest, DestSize, Fmt, Args...);
	}

	/**
	 * Helper function to write formatted output using an argument list
	 *
	 * @param Dest - destination string buffer
	 * @param DestSize - size of destination buffer
	 * @param Fmt - string to print
	 * @param Args - argument list
	 * @return number of characters written or -1 if truncated
	 */
	static FORCEINLINE int32 GetVarArgs( CharType* Dest, SIZE_T DestSize, const CharType*& Fmt, va_list ArgPtr );

private:
	static int32 VARARGS SprintfImpl(CharType* Dest, const CharType* Fmt, ...);
	static int32 VARARGS SnprintfImpl(CharType* Dest, int32 DestSize, const CharType* Fmt, ...);
};

typedef TCString<TCHAR>    FCString;
typedef TCString<ANSICHAR> FCStringAnsi;
typedef TCString<WIDECHAR> FCStringWide;

/*-----------------------------------------------------------------------------
	generic TCString implementations
-----------------------------------------------------------------------------*/

template <typename T>
int32 TCString<T>::SprintfImpl(CharType* Dest, const CharType* Fmt, ...)
{
	int32	Result = -1;
	{
		va_list ap;
		va_start(ap, Fmt);
		Result = GetVarArgs(Dest, MAX_SPRINTF, Fmt, ap);
		if (Result >= MAX_SPRINTF)
		{
			Result = -1;
		}
		va_end(ap);
	}
	return Result;
}

template <typename T>
int32 TCString<T>::SnprintfImpl(CharType* Dest, int32 DestSize, const CharType* Fmt, ...)
{
	int32	Result = -1;
	{
		va_list ap;
		va_start(ap, Fmt);
		Result = GetVarArgs(Dest, DestSize, Fmt, ap);
		if (Result >= DestSize)
		{
			Result = -1;
		}
		va_end(ap);
	}
	return Result;
}

template <typename T> FORCEINLINE
typename TCString<T>::CharType* TCString<T>::Strcpy(CharType* Dest, SIZE_T DestCount, const CharType* Src)
{
	return FPlatformString::Strcpy(Dest, DestCount, Src);
}

template <typename T> FORCEINLINE
typename TCString<T>::CharType* TCString<T>::Strncpy( CharType* Dest, const CharType* Src, int32 MaxLen )
{
	check(MaxLen > 0);
	FPlatformString::Strncpy(Dest, Src, MaxLen);
	return Dest;
}

template <typename T> FORCEINLINE
typename TCString<T>::CharType* TCString<T>::Strcat(CharType* Dest, SIZE_T DestCount, const CharType* Src)
{
	return FPlatformString::Strcat(Dest, DestCount, Src);
}

template <typename T> FORCEINLINE
int32 TCString<T>::Strcmp( const CharType* String1, const CharType* String2 )
{
	return FPlatformString::Strcmp(String1, String2);
}

template <typename T> FORCEINLINE
int32 TCString<T>::Strncmp( const CharType* String1, const CharType* String2, SIZE_T Count)
{
	return FPlatformString::Strncmp(String1, String2, Count);
}

template <typename T> FORCEINLINE
int32 TCString<T>::Stricmp( const CharType* String1, const CharType* String2 )
{
	return FPlatformString::Stricmp(String1, String2);
}

template <typename T> FORCEINLINE
int32 TCString<T>::Strnicmp( const CharType* String1, const CharType* String2, SIZE_T Count )
{
	return FPlatformString::Strnicmp(String1, String2, Count);
}

template <typename T> FORCEINLINE
int32 TCString<T>::Strlen( const CharType* String )
{
	return FPlatformString::Strlen(String);
}

template <typename T> FORCEINLINE
int32 TCString<T>::GetVarArgs( CharType* Dest, SIZE_T DestSize, const CharType*& Fmt, va_list ArgPtr )
{
	return FPlatformString::GetVarArgs(Dest, DestSize, Fmt, ArgPtr);
}

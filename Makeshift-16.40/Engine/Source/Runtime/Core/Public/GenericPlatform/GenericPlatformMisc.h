// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

#if UE_BUILD_SHIPPING
#define UE_DEBUG_BREAK() ((void)0)
#else
#define UE_DEBUG_BREAK() ((void)(FPlatformMisc::IsDebuggerPresent() && ([] () { UE_DEBUG_BREAK_IMPL(); } (), 1)))
#endif

/**
* Generic implementation for most platforms
**/
struct FGenericPlatformMisc
{
	/** Prints string to the default output */
	static void LowLevelOutputDebugString(const TCHAR *Message);

	static void VARARGS LowLevelOutputDebugStringf(const TCHAR *Format, ... );

	/** Prints string to the default output */
	static void LocalPrint( const TCHAR* Str );

	/** Return true if a debugger is present */
	FORCEINLINE static bool IsDebuggerPresent()
	{
#if UE_BUILD_SHIPPING
		return 0;
#else
		return 1; // unknown platforms return true so that they can crash into a debugger
#endif
	}

	static bool IsEnsureAllowed() { return true; }

	static void PromptForRemoteDebugging(bool bIsEnsure)
	{
	}
};

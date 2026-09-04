// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"

#define UE_DEBUG_BREAK_IMPL() PLATFORM_BREAK()

/**
 * Windows implementation of the misc OS functions
 **/
struct FWindowsPlatformMisc
	: public FGenericPlatformMisc
{
#if !UE_BUILD_SHIPPING
	static bool IsDebuggerPresent();
#endif

	static void LocalPrint(const TCHAR *Message);
};

typedef FWindowsPlatformMisc FPlatformMisc;

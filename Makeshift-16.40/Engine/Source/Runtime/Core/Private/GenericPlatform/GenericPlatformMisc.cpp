// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/GenericPlatform/GenericPlatformMisc.h"
#include "Engine/Source/Runtime/Core/Public/HAL/PlatformMisc.h"
#include "Engine/Source/Runtime/Core/Private/Misc/VarargsHelper.h"

#include <cstdio>

void FGenericPlatformMisc::LowLevelOutputDebugString( const TCHAR *Message )
{
	FPlatformMisc::LocalPrint( Message );
}

void FGenericPlatformMisc::LowLevelOutputDebugStringf(const TCHAR *Fmt, ... )
{
	GROWABLE_LOGF(
		FPlatformMisc::LowLevelOutputDebugString( Buffer );
	);
}

void FGenericPlatformMisc::LocalPrint( const TCHAR* Str )
{
	wprintf(TEXT("%ls"), Str);
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Windows/WindowsPlatformMisc.h"

#if !UE_BUILD_SHIPPING
bool FWindowsPlatformMisc::IsDebuggerPresent()
{
	return !!::IsDebuggerPresent();
}
#endif

void FWindowsPlatformMisc::LocalPrint( const TCHAR *Message )
{
	OutputDebugString(Message);
}

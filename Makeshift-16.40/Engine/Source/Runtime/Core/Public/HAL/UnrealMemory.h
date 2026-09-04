// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

#include <cstdlib>

struct FMemory
{
	//
	// C style memory allocation stubs that fall back to C runtime
	//
	static FORCEINLINE void* SystemMalloc(SIZE_T Size)
	{
		return ::malloc(Size);
	}

	static FORCEINLINE void SystemFree(void* Ptr)
	{
		::free(Ptr);
	}

	static FORCEINLINE void* Memcpy(void* Dest, const void* Src, SIZE_T Count)
	{
		return ::memcpy(Dest, Src, Count);
	}

	static FORCEINLINE void* Memset(void* Dest, uint8 Char, SIZE_T Count)
	{
		return ::memset(Dest, Char, Count);
	}

	static FORCEINLINE void* Memzero(void* Dest, SIZE_T Count)
	{
		return ::memset(Dest, 0, Count);
	}
};

// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	CoreNative.h: Native function lookup table.
=============================================================================*/

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Script.h"

struct FFrame;

/** The type of a native function callable by script */
typedef void (*FNativeFuncPtr)(UObject* Context, FFrame& TheStack, RESULT_DECL);

// This class is deliberately simple (i.e. POD) to keep generated code size down.
struct FNameNativePtrPair
{
	const char* NameUTF8;
	FNativeFuncPtr Pointer;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/GenericPlatform/GenericPlatformMemory.h"

/**
* Windows implementation of the memory OS functions
**/
struct CORE_API FWindowsPlatformMemory
	: public FGenericPlatformMemory
{
};

typedef FWindowsPlatformMemory FPlatformMemory;

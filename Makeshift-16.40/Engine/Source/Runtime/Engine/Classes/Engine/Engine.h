// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

// The kind of failure handling that GetWorldFromContextObject uses 
enum class EGetWorldErrorMode
{
	// Silently returns nullptr, the calling code is expected to handle this gracefully
	ReturnNull,

	// Raises a runtime error but still returns nullptr, the calling code is expected to handle this gracefully
	LogAndReturnNull,

	// Asserts, the calling code is not expecting to handle a failure gracefully
	Assert
};

/** Global engine pointer. Can be 0 so don't use without checking. */
inline UEngine*& GEngine = *reinterpret_cast<UEngine**>(InSDKUtils::GetImageBase() + 0x9391B80);

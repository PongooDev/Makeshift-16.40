// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/** Global engine pointer. Can be 0 so don't use without checking. */
inline UEngine*& GEngine = *reinterpret_cast<UEngine**>(InSDKUtils::GetImageBase() + 0x9391B80);

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/EnumClassFlags.h"

/**
 * Flags for channel creation.
 */
enum class EChannelCreateFlags : uint32
{
	None			= (1 << 0),
	OpenedLocally	= (1 << 1)
};

ENUM_CLASS_FLAGS(EChannelCreateFlags);

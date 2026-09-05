// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/EnumClassFlags.h"

enum class ESetChannelActorFlags : uint32
{
	None					= 0,
	SkipReplicatorCreation	= (1 << 0),
	SkipMarkActive			= (1 << 1),
};

ENUM_CLASS_FLAGS(ESetChannelActorFlags);

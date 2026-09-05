// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

#define FORCEINLINE_STATS FORCEINLINE
//#define FORCEINLINE_STATS FORCEINLINE_DEBUGGABLE
#define checkStats(x)

#if !defined(STATS)
#error "STATS must be defined as either zero or one."
#endif

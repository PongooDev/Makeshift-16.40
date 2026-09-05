// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

// Intended categories:
//	Log - This happened. What gameplay programmers may care about to debug
//	Verbose - This is why this happened. What you may turn on to debug the skill system code.
//  VeryVerbose - This is what didn't happen, and why. Extreme printf debugging
//

#if NO_LOGGING || !PLATFORM_DESKTOP

// Without logging enabled we pass ability system through to UE_LOG which only handles Fatal verbosity in NO_LOGGING
#define ABILITY_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAbilitySystem, Verbosity, Format, ##__VA_ARGS__); \
}

#else

#define ABILITY_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAbilitySystem, Verbosity, Format, ##__VA_ARGS__); \
}

#endif //NO_LOGGING

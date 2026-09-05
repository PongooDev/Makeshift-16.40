// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/** Possible state of the current match, where a match is all the gameplay that happens on a single map */
namespace MatchState
{
	inline const FName& EnteringMap = *reinterpret_cast<const FName*>(InSDKUtils::GetImageBase() + 0x945A900);			// We are entering this map, actors are not yet ticking
	inline const FName& WaitingToStart = *reinterpret_cast<const FName*>(InSDKUtils::GetImageBase() + 0x945A908);		// Actors are ticking, but the match has not yet started
	inline const FName& InProgress = *reinterpret_cast<const FName*>(InSDKUtils::GetImageBase() + 0x945A910);			// Normal gameplay is occurring. Specific games will have their own state machine inside this state
	inline const FName& WaitingPostMatch = *reinterpret_cast<const FName*>(InSDKUtils::GetImageBase() + 0x945A918);		// Match has ended so we aren't accepting new players, but actors are still ticking
	inline const FName& LeavingMap = *reinterpret_cast<const FName*>(InSDKUtils::GetImageBase() + 0x945A920);			// We are transitioning out of the map to another location
	inline const FName& Aborted = *reinterpret_cast<const FName*>(InSDKUtils::GetImageBase() + 0x945A928);				// Match has failed due to network issues or other problems, cannot continue

	// If a game needs to add additional states, you may need to override HasMatchStarted and HasMatchEnded to deal with the new states
	// Do not add any states before WaitingToStart or after WaitingPostMatch
}

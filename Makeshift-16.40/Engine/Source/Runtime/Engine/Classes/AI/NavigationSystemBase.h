// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

UENUM()
enum class FNavigationSystemRunMode : uint8
{
	InvalidMode,
	GameMode,
	EditorMode,
	SimulationMode,
	PIEMode,
	InferFromWorldMode,
};

namespace FNavigationSystem
{
	/** Creates an instance of NavigationSystem (class being specified by WorldSetting's NavigationSystemConfig)
	 *	A new instance will be created only if given WorldOwner doesn't have one yet.
	 *	The new instance will be assigned to the given WorldOwner (via SetNavigationSystem 
	 *	call) and depending on value of bInitializeForWorld the InitializeForWorld 
	 *	function will be called on the new NavigationSystem instance.
	 *	(@see UWorld.NavigationSystem)
	 *	@param RunMode if set to a valid value (other than FNavigationSystemRunMode::InvalidMode) 
	 *		will also configure the created NavigationSystem instance for that mode
	 *	@param NavigationSystemConfig is used to pick the navigation system's class and set it up. If null
	 *		then WorldOwner.WorldSettings.NavigationSystemConfig will be used
	 */
	inline void (*AddNavigationSystemToWorldOG)(UWorld& WorldOwner, const FNavigationSystemRunMode RunMode, UNavigationSystemConfig* NavigationSystemConfig, const bool bInitializeForWorld, const bool bOverridePreviousNavSys) = nullptr;
}

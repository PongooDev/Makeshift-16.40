// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/HAL/IConsoleManager.h"
#include "Engine/Source/Runtime/Engine/Classes/AI/NavigationSystemBase.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

static bool IsAthenaWorld(AWorldSettings* WorldSettings)
{
	UClass* DefaultGameModeClass = WorldSettings ? WorldSettings->DefaultGameMode.Get() : nullptr;
	return DefaultGameModeClass != nullptr && DefaultGameModeClass->IsChildOf(AFortGameModeAthena::StaticClass());
}

static UNavigationSystemConfig* ResolveNavigationSystemConfig(AWorldSettings* WorldSettings, UNavigationSystemConfig* NavigationSystemConfig)
{
	if (NavigationSystemConfig == nullptr && WorldSettings != nullptr)
	{
		NavigationSystemConfig = WorldSettings->GetNavigationSystemConfig();
	}

	if (WorldSettings == nullptr || !IsAthenaWorld(WorldSettings))
	{
		return NavigationSystemConfig;
	}

	if (NavigationSystemConfig != nullptr && !NavigationSystemConfig->IsA(UNullNavSysConfig::StaticClass()))
	{
		return NavigationSystemConfig;
	}

	UAthenaNavSystemConfig* AthenaConfig = NewObject<UAthenaNavSystemConfig>(WorldSettings, UAthenaNavSystemConfig::StaticClass());
	if (AthenaConfig == nullptr)
	{
		UE_LOG(LogNavigation, Warning, TEXT("UAthenaNavSystem::AddNavigationSystemToWorld : Failed to create an AthenaNavSystemConfig to replace the cooked-out one"));
		return NavigationSystemConfig;
	}

	AthenaConfig->bAutoSpawnMissingNavData = true;
	AthenaConfig->bUsesStreamedInNavLevel = false;
	AthenaConfig->bRebuildOnInitialUnlock = true;

	WorldSettings->SetNavigationSystemConfigOverride(AthenaConfig);

	UE_LOG(LogNavigation, Log, TEXT("UAthenaNavSystem::AddNavigationSystemToWorld : %hs carries no navigation system config, using %hs instead"), NavigationSystemConfig ? NavigationSystemConfig->GetName().c_str() : "The world settings", AthenaConfig->GetName().c_str());
	return AthenaConfig;
}

static void AllowEveryAgentOnNavigationBoundsVolumes(UWorld& WorldOwner)
{
	TArray<AActor*> NavigationBoundsVolumes;
	UGameplayStatics::GetAllActorsOfClass(&WorldOwner, ANavMeshBoundsVolume::StaticClass(), &NavigationBoundsVolumes);

	const uint16 EveryAgent = 0xFFFF;

	for (int32 Index = 0; Index < NavigationBoundsVolumes.Num(); ++Index)
	{
		ANavMeshBoundsVolume* NavigationBoundsVolume = NavigationBoundsVolumes[Index] ? NavigationBoundsVolumes[Index]->Cast<ANavMeshBoundsVolume>() : nullptr;
		if (NavigationBoundsVolume == nullptr)
		{
			continue;
		}

		UBrushComponent* BrushComponent = NavigationBoundsVolume->BrushComponent;
		const FVector Origin = BrushComponent ? BrushComponent->Bounds.Origin : FVector(0.f, 0.f, 0.f);
		const FVector Extent = BrushComponent ? BrushComponent->Bounds.BoxExtent : FVector(0.f, 0.f, 0.f);
		const uint16 SupportedAgents = *reinterpret_cast<const uint16*>(&NavigationBoundsVolume->SupportedAgents);

		if (SupportedAgents != EveryAgent)
		{
			NavigationBoundsVolume->SupportedAgents = *reinterpret_cast<const FNavAgentSelector*>(&EveryAgent);
		}

		UE_LOG(LogNavigation, Log, TEXT("UAthenaNavSystem::AddNavigationSystemToWorld : %hs of %hs supported agents 0x%04X, now 0x%04X, at %f %f %f with an extent of %f %f %f"), NavigationBoundsVolume->GetName().c_str(), NavigationBoundsVolume->Outer ? NavigationBoundsVolume->Outer->GetFullName().c_str() : "no level", SupportedAgents, *reinterpret_cast<const uint16*>(&NavigationBoundsVolume->SupportedAgents), Origin.X, Origin.Y, Origin.Z, Extent.X, Extent.Y, Extent.Z);
	}
}

void UAthenaNavSystem::BuildNavigationData(UWorld* WorldOwner)
{
	UFortNavSystem* NavigationSystem = (WorldOwner != nullptr && WorldOwner->NavigationSystem != nullptr) ? WorldOwner->NavigationSystem->Cast<UFortNavSystem>() : nullptr;
	if (NavigationSystem == nullptr)
	{
		return;
	}

	UE_LOG(LogNavigation, Log, TEXT("UAthenaNavSystem::BuildNavigationData : navigation octree locked %d, navigation building locked %d"), NavigationSystem->IsNavigationOctreeLocked(), NavigationSystem->IsNavigationBuildingLocked());

	NavigationSystem->BuildNavigationData();
}

static void PreloadBuildNavigationData()
{
	UAthenaNavSystem::GetDefaultObj()->bInitialBuildingLocked = true;

	IConsoleVariable* PreloadBuildNavigationDataVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("Preload.BuildNavigationData"));
	if (PreloadBuildNavigationDataVariable == nullptr)
	{
		UE_LOG(LogNavigation, Warning, TEXT("UAthenaNavSystem::AddNavigationSystemToWorld : Preload.BuildNavigationData is not registered, the navigation octree will be unlocked before the playlist levels are visible"));
		return;
	}

	PreloadBuildNavigationDataVariable->Set(true, ECVF_SetByCode);
}

static void AddNavigationSystemToWorldHook(UWorld& WorldOwner, const FNavigationSystemRunMode RunMode, UNavigationSystemConfig* NavigationSystemConfig, const bool bInitializeForWorld, const bool bOverridePreviousNavSys)
{
	AWorldSettings* WorldSettings = WorldOwner.GetWorldSettings();
	const bool bIsAthenaWorld = IsAthenaWorld(WorldSettings);

	if (WorldOwner.NavigationSystem == nullptr || bOverridePreviousNavSys)
	{
		if (bIsAthenaWorld)
		{
			PreloadBuildNavigationData();
		}

		NavigationSystemConfig = ResolveNavigationSystemConfig(WorldSettings, NavigationSystemConfig);
	}

	if (bInitializeForWorld && bIsAthenaWorld)
	{
		AllowEveryAgentOnNavigationBoundsVolumes(WorldOwner);
	}

	FNavigationSystem::AddNavigationSystemToWorldOG(WorldOwner, RunMode, NavigationSystemConfig, bInitializeForWorld, bOverridePreviousNavSys);

	UNavigationSystemV1* NavigationSystem = WorldOwner.NavigationSystem ? WorldOwner.NavigationSystem->Cast<UNavigationSystemV1>() : nullptr;
	UE_LOG(LogNavigation, Log, TEXT("UAthenaNavSystem::AddNavigationSystemToWorld : %hs has %hs, navigation octree locked %d, navigation building locked %d"), WorldOwner.GetName().c_str(), NavigationSystem ? NavigationSystem->GetName().c_str() : "no navigation system", NavigationSystem != nullptr && NavigationSystem->IsNavigationOctreeLocked(), NavigationSystem != nullptr && NavigationSystem->IsNavigationBuildingLocked());
}

void UAthenaNavSystem::Init()
{
	LogNavigation.SetVerbosity(ELogVerbosity::Log);

	Memory::HookDetour(ImageBase + 0x1EEA0E0, AddNavigationSystemToWorldHook, &FNavigationSystem::AddNavigationSystemToWorldOG);
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
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

static void LogSupportedAgents(UWorld& WorldOwner)
{
	UNavigationSystemV1* NavigationSystem = WorldOwner.NavigationSystem ? WorldOwner.NavigationSystem->Cast<UNavigationSystemV1>() : nullptr;
	if (NavigationSystem == nullptr)
	{
		return;
	}

	for (int32 AgentIndex = 0; AgentIndex < NavigationSystem->SupportedAgents.Num(); ++AgentIndex)
	{
		FNavDataConfig& NavConfig = NavigationSystem->SupportedAgents[AgentIndex];
		UE_LOG(LogNavigation, Log, TEXT("UAthenaNavSystem::AddNavigationSystemToWorld : SupportedAgent[%d] is %hs, navigation data class %hs, radius %f height %f"), AgentIndex, NavConfig.Name.ToString().c_str(), NavConfig.NavigationDataClass.Get() ? NavConfig.NavigationDataClass.Get()->GetName().c_str() : "none", NavConfig.AgentRadius, NavConfig.AgentHeight);
	}
}

static void AddNavigationSystemToWorldHook(UWorld& WorldOwner, const FNavigationSystemRunMode RunMode, UNavigationSystemConfig* NavigationSystemConfig, const bool bInitializeForWorld, const bool bOverridePreviousNavSys)
{
	AWorldSettings* WorldSettings = WorldOwner.GetWorldSettings();
	const bool bIsAthenaWorld = IsAthenaWorld(WorldSettings);

	if (WorldOwner.NavigationSystem == nullptr || bOverridePreviousNavSys)
	{
		NavigationSystemConfig = ResolveNavigationSystemConfig(WorldSettings, NavigationSystemConfig);
	}

	if (bInitializeForWorld && bIsAthenaWorld)
	{
		AllowEveryAgentOnNavigationBoundsVolumes(WorldOwner);
	}

	FNavigationSystem::AddNavigationSystemToWorldOG(WorldOwner, RunMode, NavigationSystemConfig, bInitializeForWorld, bOverridePreviousNavSys);

	UE_LOG(LogNavigation, Log, TEXT("UAthenaNavSystem::AddNavigationSystemToWorld : %hs has %hs"), WorldOwner.GetName().c_str(), WorldOwner.NavigationSystem ? WorldOwner.NavigationSystem->GetName().c_str() : "no navigation system");

	if (bInitializeForWorld && bIsAthenaWorld)
	{
		LogSupportedAgents(WorldOwner);
	}
}

void UAthenaNavSystem::Init()
{
	LogNavigation.SetVerbosity(ELogVerbosity::Log);

	Memory::HookDetour(ImageBase + 0x1EEA0E0, AddNavigationSystemToWorldHook, &FNavigationSystem::AddNavigationSystemToWorldOG);
}

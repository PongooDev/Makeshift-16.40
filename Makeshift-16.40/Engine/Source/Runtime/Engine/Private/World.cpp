// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	World.cpp: UWorld implementation
=============================================================================*/

#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/World.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CommandLine.h"
#include "Engine/Source/Runtime/Core/Public/Misc/Parse.h"
#include "Engine/Source/Runtime/Core/Public/HAL/LowLevelMemTracker.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/EngineLogs.h"

bool UWorld::Listen( FURL& InURL )
{
	LLM_SCOPE(ELLMTag::Networking);

	if( NetDriver )
	{
		GEngine->BroadcastNetworkFailure(this, NetDriver, ENetworkFailure::NetDriverAlreadyExists);
		return false;
	}

	// Create net driver.
	if (GEngine->CreateNamedNetDriver(this, NAME_GameNetDriver, NAME_GameNetDriver))
	{
		NetDriver = GEngine->FindNamedNetDriver(this, NAME_GameNetDriver);
		NetDriver->SetWorld(this);
		FLevelCollection* const SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);
		if (SourceCollection)
		{
			SourceCollection->SetNetDriver(NetDriver);
		}
		FLevelCollection* const StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);
		if (StaticCollection)
		{
			StaticCollection->SetNetDriver(NetDriver);
		}
	}

	if (NetDriver == nullptr)
	{
		GEngine->BroadcastNetworkFailure(this, NULL, ENetworkFailure::NetDriverCreateFailure);
		return false;
	}

	FString Error;
	if( !NetDriver->InitListen( GetNetworkNotify(), InURL, false, Error ) )
	{
		GEngine->BroadcastNetworkFailure(this, NetDriver, ENetworkFailure::NetDriverListenFailure, Error);
		UE_LOG(LogWorld, Log,  TEXT("Failed to listen: %s"), *Error );
		GEngine->DestroyNamedNetDriver(this, NetDriver->NetDriverName);
		NetDriver = nullptr;
		FLevelCollection* SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);
		if (SourceCollection)
		{
			SourceCollection->SetNetDriver(nullptr);
		}
		FLevelCollection* StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);
		if (StaticCollection)
		{
			StaticCollection->SetNetDriver(nullptr);
		}
		return false;
	}
	static const bool bLanPlay = FParse::Param(FCommandLine::Get(),TEXT("lanplay"));
	const bool bLanSpeed = bLanPlay || InURL.HasOption(TEXT("LAN"));
	if ( !bLanSpeed && (NetDriver->MaxInternetClientRate < NetDriver->MaxClientRate) && (NetDriver->MaxInternetClientRate > 2500) )
	{
		NetDriver->MaxClientRate = NetDriver->MaxInternetClientRate;
	}

	NextSwitchCountdown = NetDriver->ServerTravelPause;
	return true;
}

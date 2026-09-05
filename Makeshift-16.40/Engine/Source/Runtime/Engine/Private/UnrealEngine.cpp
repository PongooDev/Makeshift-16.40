// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
UnrealEngine.cpp: Implements the UEngine class and helpers.
=============================================================================*/

#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/EngineLogs.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"

bool UEngine::LoadMapHook(UEngine* This, FWorldContext& WorldContext, FURL& URL, class UPendingNetGame* Pending, FString& Error)
{
	bool result = LoadMapOG(This, WorldContext, URL, Pending, Error);

	URL = WorldContext.LastURL;

	// Listen for clients.
	if (result && Pending == NULL && (!GIsClient || URL.HasOption(TEXT("Listen"))))
	{
		if (!WorldContext.World()->Listen(URL))
		{
			UE_LOG(LogNet, Error, TEXT("LoadMap: failed to Listen(%s)"), *URL.ToString());
		}
	}

	return result;
}

void UEngine::Init()
{
	Memory::HookDetour(ImageBase + 0x12D4398, LoadMapHook, &LoadMapOG);
	Memory::NopFunctionCall(ImageBase + 0x32CB0FC);
}

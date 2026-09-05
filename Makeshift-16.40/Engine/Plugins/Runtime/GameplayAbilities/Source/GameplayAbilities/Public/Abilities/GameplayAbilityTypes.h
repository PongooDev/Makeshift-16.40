// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"
#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbilityTargetTypes.h"

/** Struct defining the cached data for a specific gameplay ability. This data is generally synchronized client->server in a network game. */
struct GAMEPLAYABILITIES_API FAbilityReplicatedDataCache
{
	/** What elements this activation is targeting */
	FGameplayAbilityTargetDataHandle TargetData;

	/** What tag to pass through when doing an application */
	FGameplayTag ApplicationTag;

	/** True if we've been positively confirmed our targeting, false if we don't know */
	bool bTargetConfirmed;

	/** True if we've been positively cancelled our targeting, false if we don't know */
	bool bTargetCancelled;

	/** Delegate to call whenever this is modified */
	FAbilityTargetDataSetDelegate TargetSetDelegate;

	/** Delegate to call whenever this is confirmed (without target data) */
	FSimpleMulticastDelegate TargetCancelledDelegate;

	/** Generic events that contain no payload data */
	FAbilityReplicatedData	GenericEvents[(int32) EAbilityGenericReplicatedEvent::MAX];

	/** Prediction Key when this data was set */
	FPredictionKey PredictionKey;

	FAbilityReplicatedDataCache() : bTargetConfirmed(false), bTargetCancelled(false) {}
	virtual ~FAbilityReplicatedDataCache() { }

	/** Resets any cached data, leaves delegates up */
	void Reset()
	{
		void (*Fn)(FAbilityReplicatedDataCache*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x371C284);
		Fn(this);
	}
};

static_assert(sizeof(FAbilityReplicatedDataCache) == 0x000260, "Wrong size on FAbilityReplicatedDataCache");
static_assert(offsetof(FAbilityReplicatedDataCache, TargetData) == 0x000008, "Member 'FAbilityReplicatedDataCache::TargetData' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedDataCache, ApplicationTag) == 0x000030, "Member 'FAbilityReplicatedDataCache::ApplicationTag' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedDataCache, bTargetConfirmed) == 0x000038, "Member 'FAbilityReplicatedDataCache::bTargetConfirmed' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedDataCache, bTargetCancelled) == 0x000039, "Member 'FAbilityReplicatedDataCache::bTargetCancelled' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedDataCache, TargetSetDelegate) == 0x000040, "Member 'FAbilityReplicatedDataCache::TargetSetDelegate' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedDataCache, TargetCancelledDelegate) == 0x000058, "Member 'FAbilityReplicatedDataCache::TargetCancelledDelegate' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedDataCache, GenericEvents) == 0x000070, "Member 'FAbilityReplicatedDataCache::GenericEvents' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedDataCache, PredictionKey) == 0x000250, "Member 'FAbilityReplicatedDataCache::PredictionKey' has a wrong offset!");

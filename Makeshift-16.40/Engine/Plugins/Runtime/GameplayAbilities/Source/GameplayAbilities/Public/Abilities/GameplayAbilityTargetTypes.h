// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

/** Payload for generic replicated events */
struct FAbilityReplicatedData
{
	/** Event has triggered */
	bool bTriggered;

	/** Optional Vector payload for event */
	FVector_NetQuantize100 VectorPayload;

	/** Delegate that will be called on replication */
	FSimpleMulticastDelegate Delegate;
};

static_assert(sizeof(FAbilityReplicatedData) == 0x000028, "Wrong size on FAbilityReplicatedData");
static_assert(offsetof(FAbilityReplicatedData, VectorPayload) == 0x000004, "Member 'FAbilityReplicatedData::VectorPayload' has a wrong offset!");
static_assert(offsetof(FAbilityReplicatedData, Delegate) == 0x000010, "Member 'FAbilityReplicatedData::Delegate' has a wrong offset!");

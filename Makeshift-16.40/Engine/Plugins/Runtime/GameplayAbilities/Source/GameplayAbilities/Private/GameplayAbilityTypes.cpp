// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/Abilities/GameplayAbilityTypes.h"
#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayAbilitySpec.h"

// ----------------------------------------------------

FScopedAbilityListLock::FScopedAbilityListLock(UAbilitySystemComponent& InAbilitySystemComponent)
	: AbilitySystemComponent(InAbilitySystemComponent)
{
	AbilitySystemComponent.IncrementAbilityListLock();
}

FScopedAbilityListLock::~FScopedAbilityListLock()
{
	AbilitySystemComponent.DecrementAbilityListLock();
}

// ----------------------------------------------------

TSharedPtr<FAbilityReplicatedDataCache> FGameplayAbilityReplicatedDataContainer::Find(const FGameplayAbilitySpecHandleAndPredictionKey& Key) const
{
	for (const FKeyDataPair& Pair : InUseData)
	{
		if (Pair.Key == Key)
		{
			return Pair.Value;
		}
	}

	return TSharedPtr<FAbilityReplicatedDataCache>();
}

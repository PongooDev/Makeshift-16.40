// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

/** Used to stop us from removing abilities from an ability system component while we're iterating through the abilities */
struct GAMEPLAYABILITIES_API FScopedAbilityListLock
{
	FScopedAbilityListLock(UAbilitySystemComponent& InContainer);
	~FScopedAbilityListLock();

private:
	UAbilitySystemComponent& AbilitySystemComponent;
};

#define ABILITYLIST_SCOPE_LOCK()	FScopedAbilityListLock ActiveScopeLock(*this);

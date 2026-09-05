// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/GameplayPrediction.h"

FScopedPredictionWindow::FScopedPredictionWindow(UAbilitySystemComponent* AbilitySystemComponent, FPredictionKey InPredictionKey, bool InSetReplicatedPredictionKey /*=true*/)
{
	void (*Fn)(FScopedPredictionWindow*, UAbilitySystemComponent*, FPredictionKey, bool) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x3722C98);
	Fn(this, AbilitySystemComponent, InPredictionKey, InSetReplicatedPredictionKey);
}

FScopedPredictionWindow::~FScopedPredictionWindow()
{
	void (*Fn)(FScopedPredictionWindow*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x17327F0);
	Fn(this);
}

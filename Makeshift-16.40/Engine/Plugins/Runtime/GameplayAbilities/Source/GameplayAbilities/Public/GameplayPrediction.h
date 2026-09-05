// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

// -----------------------------------------------------------------

/**
 *	A structure for allowing scoped prediction windows.
 */

struct GAMEPLAYABILITIES_API FScopedPredictionWindow
{
	/** To be called on server when a new prediction key is received from the client (In an RPC).
	 *	InSetReplicatedPredictionKey should be set to false in cases where we want a scoped prediction key but have already repped the prediction key.
	 *	(For example, cached target data will restore the prediction key that the TD was sent with, but this key was already repped down as confirmed when received)
	 **/
	FScopedPredictionWindow(UAbilitySystemComponent* AbilitySystemComponent, FPredictionKey InPredictionKey, bool InSetReplicatedPredictionKey = true);

	~FScopedPredictionWindow();

	FPredictionKey	ScopedPredictionKey;

private:

	TWeakObjectPtr<UAbilitySystemComponent> Owner;
	bool ClearScopedPredictionKey;
	bool SetReplicatedPredictionKey;
	FPredictionKey RestoreKey;
};

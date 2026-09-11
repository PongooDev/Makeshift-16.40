// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"

void AWorldSettings::SetNavigationSystemConfigOverride(UNavigationSystemConfig* NewConfig)
{
	NavigationSystemConfigOverride = NewConfig;
	if (NavigationSystemConfig)
	{
		NavigationSystemConfig->SetIsOverriden(NewConfig != nullptr && NewConfig != NavigationSystemConfig);
	}
}

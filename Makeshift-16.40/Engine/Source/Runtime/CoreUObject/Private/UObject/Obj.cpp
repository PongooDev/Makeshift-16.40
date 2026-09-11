// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"

bool UObject::CanCreateInCurrentContextHook(const UObject* Template)
{
	return true;
}

void UObject::Init()
{
	Memory::SwapVTableEntryInAllSubClasses<UObject>(27, CanCreateInCurrentContextHook);
}

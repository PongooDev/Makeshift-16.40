// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"

FString FGameplayTagContainer::ToStringSimple(bool bQuoted) const
{
	FString RetString;
	for (int i = 0; i < GameplayTags.Num(); ++i)
	{
		if (bQuoted)
		{
			RetString += TEXT("\"");
		}
		RetString += GameplayTags[i].ToString();
		if (bQuoted)
		{
			RetString += TEXT("\"");
		}

		if (i < GameplayTags.Num() - 1)
		{
			RetString += TEXT(", ");
		}
	}
	return RetString;
}

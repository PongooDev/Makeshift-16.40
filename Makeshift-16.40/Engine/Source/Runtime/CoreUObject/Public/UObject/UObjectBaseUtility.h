// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	UObjectBaseUtility.h: Unreal UObject functions that only depend on UObjectBase
=============================================================================*/

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/**
 * Returns the name of this object (with no path information)
 * @param Object object to retrieve the name for; NULL gives "None"
 * @return Name of the object.
*/
FORCEINLINE FString GetNameSafe(const UObject *Object)
{
	if( Object == NULL )
	{
		return TEXT("None");
	}
	else
	{
		FString ResultString;
		Object->GetName(ResultString);
		return ResultString;
	}
}

#define SCOPE_CYCLE_UOBJECT(Name, Object)

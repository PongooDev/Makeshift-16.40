// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/**
 * It should be possible to provide a generic implementation as long as a threadID is provided. We don't do that yet.
 */
struct FGenericPlatformTLS
{
	/**
	 * Return false if this is an invalid TLS slot
	 * @param SlotIndex the TLS index to check
	 * @return true if this looks like a valid slot
	 */
	static FORCEINLINE bool IsValidTlsSlot(uint32 SlotIndex)
	{
		return SlotIndex != 0xFFFFFFFF;
	}
};

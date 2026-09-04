// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/**
 * This is the Windows version of a critical section. It uses an aggregate
 * CRITICAL_SECTION to implement its locking.
 */
class FWindowsCriticalSection
{
	/**
	 * The windows specific critical section
	 */
	CRITICAL_SECTION CriticalSection;

public:

	/**
	 * Constructor that initializes the aggregated critical section
	 */
	FORCEINLINE FWindowsCriticalSection()
	{
		CA_SUPPRESS(28125);
		InitializeCriticalSection(&CriticalSection);
		SetCriticalSectionSpinCount(&CriticalSection,4000);
	}

	/**
	 * Destructor cleaning up the critical section
	 */
	FORCEINLINE ~FWindowsCriticalSection()
	{
		DeleteCriticalSection(&CriticalSection);
	}

	/**
	 * Locks the critical section
	 */
	FORCEINLINE void Lock()
	{
		EnterCriticalSection(&CriticalSection);
	}

	/**
	 * Attempt to take a lock and returns whether or not a lock was taken.
	 *
	 * @return true if a lock was taken, false otherwise.
	 */
	FORCEINLINE bool TryLock()
	{
		if (TryEnterCriticalSection(&CriticalSection))
		{
			return true;
		}
		return false;
	}

	/**
	 * Releases the lock on the critical section
	 */
	FORCEINLINE void Unlock()
	{
		LeaveCriticalSection(&CriticalSection);
	}

private:
	FWindowsCriticalSection(const FWindowsCriticalSection&);
	FWindowsCriticalSection& operator=(const FWindowsCriticalSection&);
};

typedef FWindowsCriticalSection FCriticalSection;

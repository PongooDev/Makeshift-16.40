// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"
#include "Engine/Source/Runtime/Core/Public/HAL/CriticalSection.h"

/**
 * Implements a scope lock.
 *
 * <code>
 *	{
 *		// Synchronize thread access to the following data
 *		FScopeLock ScopeLock(SynchObject);
 *		// Access data that is shared among multiple threads
 *		...
 *		// When ScopeLock goes out of scope, other threads can access data
 *	}
 * </code>
 */
class FScopeLock
{
public:

	/**
	 * Constructor that performs a lock on the synchronization object
	 *
	 * @param InSynchObject The synchronization object to manage
	 */
	FScopeLock( FCriticalSection* InSynchObject )
		: SynchObject(InSynchObject)
	{
		check(SynchObject);
		SynchObject->Lock();
	}

	/** Destructor that performs a release on the synchronization object. */
	~FScopeLock()
	{
		Unlock();
	}

	void Unlock()
	{
		if(SynchObject)
		{
			SynchObject->Unlock();
			SynchObject = nullptr;
		}
	}

private:

	/** Default constructor (hidden on purpose). */
	FScopeLock();

	/** Copy constructor( hidden on purpose). */
	FScopeLock(const FScopeLock& InScopeLock);

	/** Assignment operator (hidden on purpose). */
	FScopeLock& operator=( FScopeLock& InScopeLock )
	{
		return *this;
	}

private:

	// Holds the synchronization object to aggregate and scope manage.
	FCriticalSection* SynchObject;
};

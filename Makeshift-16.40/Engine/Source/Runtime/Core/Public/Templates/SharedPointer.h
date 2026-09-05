// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Templates/SharedPointerInternals.h"


/**
 * TSharedRef is a non-nullable, non-intrusive reference-counted authoritative object reference.
 *
 * This shared reference will be conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
 */
template< class ObjectType, ESPMode Mode >
class TSharedRef
{
public:

	/**
	 * Returns a C++ reference to the object this shared reference is referencing
	 *
	 * @return  The object owned by this shared reference
	 */
	FORCEINLINE ObjectType& Get() const
	{
		// Should never be nullptr as TSharedRef is never nullable
		checkSlow( IsValid() );
		return *Object;
	}

	/**
	 * Dereference operator returns a reference to the object this shared pointer points to
	 *
	 * @return  Reference to the object
	 */
	FORCEINLINE ObjectType& operator*() const
	{
		// Should never be nullptr as TSharedRef is never nullable
		checkSlow( IsValid() );
		return *Object;
	}

	/**
	 * Arrow operator returns a pointer to this shared reference's object
	 *
	 * @return  Returns a pointer to the object referenced by this shared reference
	 */
	FORCEINLINE ObjectType* operator->() const
	{
		// Should never be nullptr as TSharedRef is never nullable
		checkSlow( IsValid() );
		return Object;
	}

private:

	/**
	 * Checks to see if this shared reference is actually pointing to an object.
	 * NOTE: This validity test is intentionally private because shared references must always be valid.
	 *
	 * @return  True if the shared reference is valid and can be dereferenced
	 */
	FORCEINLINE const bool IsValid() const
	{
		return Object != nullptr;
	}

private:

	/** The object we're holding a reference to.  Can be nullptr. */
	ObjectType* Object;

	/** Interface to the reference counter for this object.  Note that the actual reference
		controller object is shared by all shared and weak pointers that refer to the object */
	SharedPointerInternals::FSharedReferencer< Mode > SharedReferenceCount;
};


/**
 * TSharedPtr is a non-intrusive reference-counted authoritative object pointer.  This shared pointer
 * will be conditionally thread-safe when the optional Mode template argument is set to ThreadSafe.
 */
template< class ObjectType, ESPMode Mode >
class TSharedPtr
{
public:

	/**
	 * Constructs an empty shared pointer
	 */
	// NOTE: FNullTag parameter is an Unreal extension to standard shared_ptr behavior
	FORCEINLINE TSharedPtr( SharedPointerInternals::FNullTag* = nullptr )
		: Object( nullptr )
		, SharedReferenceCount()
	{ }

	/**
	 * Returns the object referenced by this pointer, or nullptr if no object is reference
	 *
	 * @return  The object owned by this shared pointer, or nullptr
	 */
	FORCEINLINE ObjectType* Get() const
	{
		return Object;
	}

	/**
	 * Checks to see if this shared pointer is actually pointing to an object
	 *
	 * @return  True if the shared pointer is valid and can be dereferenced
	 */
	FORCEINLINE explicit operator bool() const
	{
		return Object != nullptr;
	}

	/**
	 * Checks to see if this shared pointer is actually pointing to an object
	 *
	 * @return  True if the shared pointer is valid and can be dereferenced
	 */
	FORCEINLINE const bool IsValid() const
	{
		return Object != nullptr;
	}

	/**
	 * Dereference operator returns a reference to the object this shared pointer points to
	 *
	 * @return  Reference to the object
	 */
	FORCEINLINE ObjectType& operator*() const
	{
		check( IsValid() );
		return *Object;
	}

	/**
	 * Arrow operator returns a pointer to the object this shared pointer references
	 *
	 * @return  Returns a pointer to the object referenced by this shared pointer
	 */
	FORCEINLINE ObjectType* operator->() const
	{
		check( IsValid() );
		return Object;
	}

private:

	/** The object we're holding a reference to.  Can be nullptr. */
	ObjectType* Object;

	/** Interface to the reference counter for this object.  Note that the actual reference
		controller object is shared by all shared and weak pointers that refer to the object */
	SharedPointerInternals::FSharedReferencer< Mode > SharedReferenceCount;
};

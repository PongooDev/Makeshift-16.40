// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/** Default behavior. */
#define	FORCE_THREADSAFE_SHAREDPTRS PLATFORM_WEAKLY_CONSISTENT_MEMORY

/**
 * ESPMode is used select between either 'fast' or 'thread safe' shared pointer types.
 * This is only used by templates at compile time to generate one code path or another.
 */
enum class ESPMode
{
	/** Forced to be not thread-safe. */
	NotThreadSafe = 0,

	/**
	 *	Fast, doesn't ever use atomic interlocks.
	 *	Some code requires that all shared pointers are thread-safe.
	 *	It's better to change it here, instead of replacing ESPMode::Fast to ESPMode::ThreadSafe throughout the code.
	 */
	Fast = FORCE_THREADSAFE_SHAREDPTRS ? 1 : 0,

	/** Conditionally thread-safe, never spin locks, but slower */
	ThreadSafe = 1
};


// Forward declarations.  Note that in the interest of fast performance, thread safety
// features are mostly turned off (Mode = ESPMode::Fast).  If you need to access your
// object on multiple threads, you should use ESPMode::ThreadSafe!
template< class ObjectType, ESPMode Mode = ESPMode::Fast > class TSharedRef;
template< class ObjectType, ESPMode Mode = ESPMode::Fast > class TSharedPtr;
template< class ObjectType, ESPMode Mode = ESPMode::Fast > class TWeakPtr;
template< class ObjectType, ESPMode Mode = ESPMode::Fast > class TSharedFromThis;


/**
 * SharedPointerInternals contains internal workings of shared and weak pointers.  You should
 * hopefully never have to use anything inside this namespace directly.
 */
namespace SharedPointerInternals
{
	/** Dummy structures used internally as template arguments for typecasts */
	struct FStaticCastTag {};
	struct FConstCastTag {};

	// NOTE: The following is an Unreal extension to standard shared_ptr behavior
	struct FNullTag {};


	class FReferenceControllerBase
	{
	public:
		/** Constructor */
		FORCEINLINE explicit FReferenceControllerBase()
			: SharedReferenceCount(1)
			, WeakReferenceCount(1)
		{ }

		// NOTE: The primary reason these reference counters are 32-bit values (and not 16-bit to save
		//       memory), is that atomic operations require at least 32-bit objects.

		/** Number of shared references to this object.  When this count reaches zero, the associated object
			will be destroyed (even if there are still weak references!) */
		int32 SharedReferenceCount;

		/** Number of weak references to this object.  If there are any shared references, that counts as one
		   weak reference too. */
		int32 WeakReferenceCount;

		/** Destroys the object associated with this reference counter.  */
		virtual void DestroyObject() = 0;

		virtual ~FReferenceControllerBase()
		{
		}

	private:
		FReferenceControllerBase( FReferenceControllerBase const& );
		FReferenceControllerBase& operator=( FReferenceControllerBase const& );
	};

	/**
	 * FSharedReferencer is a wrapper around a pointer to a reference controller that is used by either a
	 * TSharedRef or a TSharedPtr to keep track of a referenced object's lifetime
	 */
	template< ESPMode Mode >
	class FSharedReferencer
	{
	public:

		/** Constructor for an empty shared referencer object */
		FORCEINLINE FSharedReferencer()
			: ReferenceController( nullptr )
		{ }

		/**
		 * Tests to see whether or not this shared counter contains a valid reference
		 *
		 * @return  True if reference is valid
		 */
		FORCEINLINE const bool IsValid() const
		{
			return ReferenceController != nullptr;
		}

	private:

		// Expose access to ReferenceController to FWeakReferencer
		template< ESPMode OtherMode > friend class FWeakReferencer;

	private:

		/** Pointer to the reference controller for the object a shared reference/pointer is referencing */
		FReferenceControllerBase* ReferenceController;
	};
}

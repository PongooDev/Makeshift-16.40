// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"
#include "Engine/Source/Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Engine/Source/Runtime/Core/Public/Containers/ContainerAllocationPolicies.h"
#include "Engine/Source/Runtime/Core/Public/Math/UnrealMathUtility.h"
#include "Engine/Source/Runtime/Core/Public/Math/NumericLimits.h"
#include "Engine/Source/Runtime/Core/Public/Templates/AlignmentTemplates.h"
#include "Engine/Source/Runtime/Core/Public/HAL/ThreadSingleton.h"

// Enums for specifying memory allocation type.
enum EMemZeroed
{
	MEM_Zeroed = 1
};


enum EMemOned
{
	MEM_Oned = 1
};


class CORE_API FPageAllocator
{
public:
	enum
	{
		PageSize = 64 * 1024,
		SmallPageSize = 1024-16 // allow a little extra space for allocator headers, etc
	};

	static FPageAllocator& Get();

	void* Alloc();
	void Free(void* Mem);
	void* AllocSmall();
	void FreeSmall(void* Mem);
	uint64 BytesUsed();
	uint64 BytesFree();
	void LatchProtectedMode();

private:
	FPageAllocator() {}
};


/**
 * Simple linear-allocation memory stack.
 * Items are allocated via PushBytes() or the specialized operator new()s.
 * Items are freed en masse by using FMemMark to Pop() them.
 **/
class CORE_API FMemStackBase
{
public:

	FMemStackBase(int32 InMinMarksToAlloc = 1)
		: Top(nullptr)
		, End(nullptr)
		, TopChunk(nullptr)
		, TopMark(nullptr)
		, NumMarks(0)
		, MinMarksToAlloc(InMinMarksToAlloc)
	{
	}

	~FMemStackBase()
	{
		check((GIsCriticalError || !NumMarks));
		FreeChunks(nullptr);
	}

	FORCEINLINE uint8* PushBytes(int32 AllocSize, int32 Alignment)
	{
		return (uint8*)Alloc(AllocSize, FMath::Max(AllocSize >= 16 ? (int32)16 : (int32)8, Alignment));
	}

	FORCEINLINE void* Alloc(int32 AllocSize, int32 Alignment)
	{
		// Debug checks.
		checkSlow(AllocSize>=0);
		checkSlow((Alignment&(Alignment-1))==0);
		checkSlow(Top<=End);
		checkSlow(NumMarks >= MinMarksToAlloc);

		// Try to get memory from the current chunk.
		uint8* Result = Align( Top, Alignment );
		uint8* NewTop = Result + AllocSize;

		// Make sure we didn't overflow.
		if ( NewTop <= End )
		{
			Top	= NewTop;
		}
		else
		{
			// We'd pass the end of the current chunk, so allocate a new one.
			AllocateNewChunk( AllocSize + Alignment );
			Result = Align( Top, Alignment );
			NewTop = Result + AllocSize;
			Top = NewTop;
		}
		return Result;
	}

	/** return true if this stack is empty. */
	FORCEINLINE bool IsEmpty() const
	{
		return TopChunk == nullptr;
	}

	FORCEINLINE void Flush()
	{
		check(!NumMarks && !MinMarksToAlloc);
		FreeChunks(nullptr);
	}
	FORCEINLINE int32 GetNumMarks()
	{
		return NumMarks;
	}
	/** @return the number of bytes allocated for this FMemStack that are currently in use. */
	int32 GetByteCount() const;

	// Returns true if the pointer was allocated using this allocator
	bool ContainsPointer(const void* Pointer) const;

	// Friends.
	friend class FMemMark;
	friend void* operator new(size_t Size, FMemStackBase& Mem, int32 Count, int32 Align);
	friend void* operator new(size_t Size, FMemStackBase& Mem, EMemZeroed Tag, int32 Count, int32 Align);
	friend void* operator new(size_t Size, FMemStackBase& Mem, EMemOned Tag, int32 Count, int32 Align);
	friend void* operator new[](size_t Size, FMemStackBase& Mem, int32 Count, int32 Align);
	friend void* operator new[](size_t Size, FMemStackBase& Mem, EMemZeroed Tag, int32 Count, int32 Align);
	friend void* operator new[](size_t Size, FMemStackBase& Mem, EMemOned Tag, int32 Count, int32 Align);

	// Types.
	struct FTaggedMemory
	{
		FTaggedMemory* Next;
		int32 DataSize;

		uint8 *Data() const
		{
			return ((uint8*)this) + sizeof(FTaggedMemory);
		}
	};

private:

	/**
	 * Allocate a new chunk of memory of at least MinSize size,
	 * updates the memory stack's Chunks table and ActiveChunks counter.
	 */
	void AllocateNewChunk( int32 MinSize );

	/** Frees the chunks above the specified chunk on the stack. */
	void FreeChunks( FTaggedMemory* NewTopChunk );

	// Variables.
	uint8*			Top;				// Top of current chunk (Top<=End).
	uint8*			End;				// End of current chunk.
	FTaggedMemory*	TopChunk;			// Only chunks 0..ActiveChunks-1 are valid.

	/** The top mark on the stack. */
	class FMemMark*	TopMark;

	/** The number of marks on this stack. */
	int32 NumMarks;

	/** Used for a checkSlow. Most stacks require a mark to allocate. Command lists don't because they never mark, only flush*/
	int32 MinMarksToAlloc;
};


class CORE_API FMemStack : public TThreadSingleton<FMemStack>, public FMemStackBase
{
};


/*-----------------------------------------------------------------------------
	FMemStack templates.
-----------------------------------------------------------------------------*/

// Operator new for typesafe memory stack allocation.
template <class T> inline T* New(FMemStackBase& Mem, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	return (T*)Mem.PushBytes( Count*sizeof(T), Align );
}
template <class T> inline T* NewZeroed(FMemStackBase& Mem, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	uint8* Result = Mem.PushBytes( Count*sizeof(T), Align );
	FMemory::Memzero( Result, Count*sizeof(T) );
	return (T*)Result;
}
template <class T> inline T* NewOned(FMemStackBase& Mem, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	uint8* Result = Mem.PushBytes( Count*sizeof(T), Align );
	FMemory::Memset( Result, 0xff, Count*sizeof(T) );
	return (T*)Result;
}


/*-----------------------------------------------------------------------------
	FMemStack operator new's.
-----------------------------------------------------------------------------*/

// Operator new for typesafe memory stack allocation.
inline void* operator new(size_t Size, FMemStackBase& Mem, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	// Get uninitialized memory.
	const size_t SizeInBytes = Size * Count;
	checkSlow(SizeInBytes <= (size_t)TNumericLimits<int32>::Max());
	return Mem.PushBytes( (int32)SizeInBytes, Align );
}
inline void* operator new(size_t Size, FMemStackBase& Mem, EMemZeroed Tag, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	// Get zero-filled memory.
	const size_t SizeInBytes = Size * Count;
	checkSlow(SizeInBytes <= (size_t)TNumericLimits<int32>::Max());
	uint8* Result = Mem.PushBytes( (int32)SizeInBytes, Align );
	FMemory::Memzero( Result, SizeInBytes );
	return Result;
}
inline void* operator new(size_t Size, FMemStackBase& Mem, EMemOned Tag, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	// Get one-filled memory.
	const size_t SizeInBytes = Size * Count;
	checkSlow(SizeInBytes <= (size_t)TNumericLimits<int32>::Max());
	uint8* Result = Mem.PushBytes( (int32)SizeInBytes, Align );
	FMemory::Memset( Result, 0xff, SizeInBytes );
	return Result;
}
inline void* operator new[](size_t Size, FMemStackBase& Mem, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	// Get uninitialized memory.
	const size_t SizeInBytes = Size * Count;
	checkSlow(SizeInBytes <= (size_t)TNumericLimits<int32>::Max());
	return Mem.PushBytes( (int32)SizeInBytes, Align );
}
inline void* operator new[](size_t Size, FMemStackBase& Mem, EMemZeroed Tag, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	// Get zero-filled memory.
	const size_t SizeInBytes = Size * Count;
	checkSlow(SizeInBytes <= (size_t)TNumericLimits<int32>::Max());
	uint8* Result = Mem.PushBytes( (int32)SizeInBytes, Align );
	FMemory::Memzero( Result, SizeInBytes );
	return Result;
}
inline void* operator new[](size_t Size, FMemStackBase& Mem, EMemOned Tag, int32 Count = 1, int32 Align = DEFAULT_ALIGNMENT)
{
	// Get one-filled memory.
	const size_t SizeInBytes = Size * Count;
	checkSlow(SizeInBytes <= (size_t)TNumericLimits<int32>::Max());
	uint8* Result = Mem.PushBytes( (int32)SizeInBytes, Align );
	FMemory::Memset( Result, 0xff, SizeInBytes );
	return Result;
}


/**
 * FMemMark marks a top-of-stack position in the memory stack.
 * When the marker is constructed or initialized with a particular memory
 * stack, it saves the stack's current position. When marker is popped, it
 * pops all items that were added to the stack subsequent to initialization.
 */
class FMemMark
{
public:
	// Constructors.
	FMemMark(FMemStackBase& InMem)
	:	Mem(InMem)
	,	Top(InMem.Top)
	,	SavedChunk(InMem.TopChunk)
	,	bPopped(false)
	,	NextTopmostMark(InMem.TopMark)
	{
		Mem.TopMark = this;

		// Track the number of outstanding marks on the stack.
		Mem.NumMarks++;
	}

	/** Destructor. */
	~FMemMark()
	{
		Pop();
	}

	/** Free the memory allocated after the mark was created. */
	void Pop()
	{
		if(!bPopped)
		{
			check(Mem.TopMark == this);
			bPopped = true;

			// Track the number of outstanding marks on the stack.
			--Mem.NumMarks;

			// Unlock any new chunks that were allocated.
			if( SavedChunk != Mem.TopChunk )
				Mem.FreeChunks( SavedChunk );

			// Restore the memory stack's state.
			Mem.Top = Top;
			Mem.TopMark = NextTopmostMark;

			// Ensure that the mark is only popped once by clearing the top pointer.
			Top = nullptr;
		}
	}

private:

	// Implementation variables.
	FMemStackBase& Mem;
	uint8* Top;
	FMemStackBase::FTaggedMemory* SavedChunk;
	bool bPopped;
	FMemMark* NextTopmostMark;
};

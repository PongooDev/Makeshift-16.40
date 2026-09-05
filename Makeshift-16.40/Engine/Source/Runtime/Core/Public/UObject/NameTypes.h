// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/UObject/UnrealNames.h"

/** Opaque id to a deduplicated name */
struct FNameEntryId
{
	FNameEntryId() : Value(0) {}

	/** Fast non-alphabetical order that is only stable during this process' lifetime */
	int32 CompareFast(FNameEntryId Rhs) const { return Value - Rhs.Value; };
	bool FastLess(FNameEntryId Rhs) const { return CompareFast(Rhs) < 0; }

	/** Fast non-alphabetical order that is only stable during this process' lifetime */
	bool operator<(FNameEntryId Rhs) const { return Value < Rhs.Value; }

	/** Fast non-alphabetical order that is only stable during this process' lifetime */
	bool operator>(FNameEntryId Rhs) const { return Rhs.Value < Value; }
	bool operator==(FNameEntryId Rhs) const { return Value == Rhs.Value; }
	bool operator!=(FNameEntryId Rhs) const { return Value != Rhs.Value; }

	explicit operator bool() const { return Value != 0; }

	/** Get process specific integer */
	uint32 ToUnstableInt() const { return Value; }

	/** Create from unstable int produced by this process */
	CORE_API static FNameEntryId FromUnstableInt(uint32 UnstableInt);

	FORCEINLINE static FNameEntryId FromEName(EName Ename)
	{
		return Ename == NAME_None ? FNameEntryId() : FromValidEName(Ename);
	}

private:
	uint32 Value;

	CORE_API static FNameEntryId FromValidEName(EName Ename);
};

/** Externally, the instance number to represent no instance number is NAME_NO_NUMBER,
    but internally, we add 1 to indices, so we use this #define internally for
	zero'd memory initialization will still make NAME_None as expected */
#define NAME_NO_NUMBER_INTERNAL	0

/** Conversion routines between external representations and internal */
#define NAME_INTERNAL_TO_EXTERNAL(x) (x - 1)
#define NAME_EXTERNAL_TO_INTERNAL(x) (x + 1)

/** Special value for an FName with no number */
#define NAME_NO_NUMBER NAME_INTERNAL_TO_EXTERNAL(NAME_NO_NUMBER_INTERNAL)

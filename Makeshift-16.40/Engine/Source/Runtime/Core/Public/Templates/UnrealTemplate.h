// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/*-----------------------------------------------------------------------------
	Standard templates.
-----------------------------------------------------------------------------*/

/**
 * Chooses between the two parameters based on whether the first is nullptr or not.
 * @return If the first parameter provided is non-nullptr, it is returned; otherwise the second parameter is returned.
 */
template<typename ReferencedType>
FORCEINLINE ReferencedType* IfAThenAElseB(ReferencedType* A,ReferencedType* B)
{
	const PTRINT IntA = reinterpret_cast<PTRINT>(A);
	const PTRINT IntB = reinterpret_cast<PTRINT>(B);

	const PTRINT MaskB = -(!IntA);

	return reinterpret_cast<ReferencedType*>(IntA | (MaskB & IntB));
}

template <typename T, uint32 N>
char (&ArrayCountHelper(const T (&)[N]))[N + 1];

// Number of elements in an array.
#define UE_ARRAY_COUNT( array ) (sizeof(ArrayCountHelper(array)) - 1)

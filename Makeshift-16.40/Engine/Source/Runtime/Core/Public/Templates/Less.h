// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Containers/ContainersFwd.h"
#include "Engine/Source/Runtime/Core/Public/Templates/UnrealTemplate.h"

/**
 * Binary predicate class for sorting elements in order.  Assumes < operator is defined for the template type.
 * Forward declaration exists in ContainersFwd.h
 *
 * See: http://en.cppreference.com/w/cpp/utility/functional/less
 */
template <typename T /*= void */>
struct TLess
{
	FORCEINLINE bool operator()(const T& A, const T& B) const
	{
		return A < B;
	}
};

template <>
struct TLess<void>
{
	template <typename T, typename U>
	FORCEINLINE bool operator()(T&& A, U&& B) const
	{
		return Forward<T>(A) < Forward<U>(B);
	}
};

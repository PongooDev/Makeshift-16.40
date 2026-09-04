// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/HAL/PreprocessorHelpers.h"

#if UE_BUILD_DEBUG
	#define FORCEINLINE_DEBUGGABLE FORCEINLINE_DEBUGGABLE_ACTUAL
#else
	#define FORCEINLINE_DEBUGGABLE FORCEINLINE
#endif

#define INDEX_NONE -1

#ifndef CA_ASSUME
	#define CA_ASSUME( Expr )
#endif
#ifndef CA_CONSTANT_IF
	#define CA_CONSTANT_IF(Condition) if (Condition)
#endif
#ifndef CA_SUPPRESS
	#define CA_SUPPRESS( WarningNumber )
#endif

#ifndef UE_NONCOPYABLE
/**
* Makes a type non-copyable and non-movable by deleting copy/move constructors and assignment/move operators.
* The macro should be placed in the public section of the type for better compiler diagnostic messages.
* Example usage:
*
*	class FMyClassName
*	{
*	public:
*		UE_NONCOPYABLE(FMyClassName)
*		FMyClassName() = default;
*	};
*/
	#define UE_NONCOPYABLE(TypeName) \
		TypeName(TypeName&&) = delete; \
		TypeName(const TypeName&) = delete; \
		TypeName& operator=(const TypeName&) = delete; \
		TypeName& operator=(TypeName&&) = delete;
#endif

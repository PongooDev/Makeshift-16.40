// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/Misc/Build.h"
#include "Engine/Source/Runtime/Core/Public/HAL/PreprocessorHelpers.h"

#include "Engine/Source/Runtime/Core/Public/Windows/WindowsPlatform.h"

//------------------------------------------------------------------
// Finalize define setup
//------------------------------------------------------------------

// Base defines, these have defaults
#ifndef PLATFORM_EXCEPTIONS_DISABLED
	#define PLATFORM_EXCEPTIONS_DISABLED	0
#endif
#ifndef PLATFORM_SUPPORTS_COLORIZED_OUTPUT_DEVICE
	#define PLATFORM_SUPPORTS_COLORIZED_OUTPUT_DEVICE 0
#endif

#ifndef VARARGS
	#define VARARGS									/* Functions with variable arguments */
#endif
#ifndef CDECL
	#define CDECL									/* Standard C function */
#endif
#ifndef STDCALL
	#define STDCALL									/* Standard calling convention */
#endif
#ifndef FORCEINLINE
	#define FORCEINLINE 							/* Force code to be inline */
#endif
#ifndef FORCENOINLINE
	#define FORCENOINLINE 							/* Force code to NOT be inline */
#endif
#ifndef RESTRICT
	#define RESTRICT __restrict						/* no alias hint */
#endif

// Branch prediction hints
#ifndef LIKELY						/* Hints compiler that expression is likely to be true, much softer than UE_ASSUME - allows (penalized by worse performance) expression to be false */
	#define LIKELY(x)			(x)
#endif

#ifndef UNLIKELY					// Hints compiler that expression is unlikely to be true, allows (penalized by worse performance) expression to be true
	#define UNLIKELY(x)			(x)
#endif

// Optimization macros (uses __pragma to enable inside a #define).
#ifndef PRAGMA_DISABLE_OPTIMIZATION_ACTUAL
	#define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL
	#define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL
#endif

#ifndef FORCEINLINE_DEBUGGABLE_ACTUAL
	#define FORCEINLINE_DEBUGGABLE_ACTUAL inline
#endif

// String constants
#ifndef LINE_TERMINATOR
	#define LINE_TERMINATOR TEXT("\n")
#endif
#ifndef LINE_TERMINATOR_ANSI
	#define LINE_TERMINATOR_ANSI "\n"
#endif

// Tells the compiler to put the decorated function in a certain section (aka. segment) of the executable.
#ifndef PLATFORM_CODE_SECTION
	#define PLATFORM_CODE_SECTION(Name)
#endif

#ifndef PLATFORM_BREAK
	#define PLATFORM_BREAK() (__debugbreak())
#endif

#ifndef CORE_API
	#define CORE_API
#endif
#ifndef ENGINE_API
	#define ENGINE_API
#endif

#ifndef UE_DEPRECATED
	#define UE_DEPRECATED(Version, Message) [[deprecated(Message " Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.")]]
#endif

#ifndef TEXT
	#define TEXT_PASTE(x) L ## x
	#define TEXT(x) TEXT_PASTE(x)
#endif

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/Misc/Build.h"
#include "Engine/Source/Runtime/Core/Public/HAL/PreprocessorHelpers.h"

#include "Engine/Source/Runtime/Core/Public/Windows/WindowsPlatformCompilerPreSetup.h"
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

#ifndef PLATFORM_CPU_X86_FAMILY
	#define PLATFORM_CPU_X86_FAMILY	1
#endif

#ifndef PLATFORM_CPU_ARM_FAMILY
	#define PLATFORM_CPU_ARM_FAMILY	0
#endif

#ifndef PLATFORM_WEAKLY_CONSISTENT_MEMORY
	#define PLATFORM_WEAKLY_CONSISTENT_MEMORY PLATFORM_CPU_ARM_FAMILY
#endif

#ifndef CONSTEXPR
	#define CONSTEXPR constexpr
#endif

// Alignment.
#ifndef GCC_PACK
	#define GCC_PACK(n)
#endif
#ifndef GCC_ALIGN
	#define GCC_ALIGN(n)
#endif
#ifndef MS_ALIGN
	#define MS_ALIGN(n)
#endif

// Defines for the availibility of the various levels of vector intrinsics.
// These may be set from UnrealBuildTool, otherwise each platform-specific platform.h is expected to set them appropriately.
#ifndef PLATFORM_ENABLE_VECTORINTRINSICS
	#define PLATFORM_ENABLE_VECTORINTRINSICS	0
#endif
// If PLATFORM_MAYBE_HAS_### is 1, then ### intrinsics are compilable.
// This does not guarantee that the intrinsics are runnable on all instances of the platform however; a runtime check such as cpuid may be required to confirm availability.
// If PLATFORM_ALWAYS_HAS_### is 1, then ## intrinsics will compile and run on all instances of the platform.  PLATFORM_ALWAYS_HAS_### == 1 implies PLATFORM_MAYBE_HAS_### == 1.
#ifndef PLATFORM_MAYBE_HAS_SSE4_1
	#define PLATFORM_MAYBE_HAS_SSE4_1			0
#endif
#ifndef PLATFORM_ALWAYS_HAS_SSE4_1
	#define PLATFORM_ALWAYS_HAS_SSE4_1			0
#endif
#ifndef PLATFORM_MAYBE_HAS_AVX
	#define PLATFORM_MAYBE_HAS_AVX				0
#endif
#ifndef PLATFORM_ALWAYS_HAS_AVX
	#define PLATFORM_ALWAYS_HAS_AVX				0
#endif


#ifndef PLATFORM_HAS_CPUID
	#if defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__x86_64__) || defined (__amd64__)
		#define PLATFORM_HAS_CPUID				1
	#else
		#define PLATFORM_HAS_CPUID				0
	#endif
#endif
#ifndef PLATFORM_ENABLE_POPCNT_INTRINSIC
	// PC is disabled by default, but linux and mac are enabled
	// if your min spec is an AMD cpu mid-2007 or Intel 2008, you should enable this
	#define PLATFORM_ENABLE_POPCNT_INTRINSIC 0
#endif
#ifndef PLATFORM_ENABLE_VECTORINTRINSICS_NEON
	#define PLATFORM_ENABLE_VECTORINTRINSICS_NEON	0
#endif

/* Use before a function declaration to warn that callers should not ignore the return value */
#if !defined(UE_NODISCARD) && defined(__has_cpp_attribute)
	#if __has_cpp_attribute(nodiscard)
		#define UE_NODISCARD [[nodiscard]]
	#endif
#endif
#ifndef UE_NODISCARD
	#define UE_NODISCARD
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

// Prefetch
#ifndef PLATFORM_CACHE_LINE_SIZE
	#define PLATFORM_CACHE_LINE_SIZE	64
#endif

#ifndef CORE_API
	#define CORE_API
#endif
#ifndef ENGINE_API
	#define ENGINE_API
#endif
#ifndef GAMEPLAYABILITIES_API
	#define GAMEPLAYABILITIES_API
#endif

#ifndef UE_DEPRECATED
	#define UE_DEPRECATED(Version, Message) [[deprecated(Message " Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.")]]
#endif

#ifndef TEXT
	#define TEXT_PASTE(x) L ## x
	#define TEXT(x) TEXT_PASTE(x)
#endif

/// The type of the NULL constant.
typedef decltype(NULL)	TYPE_OF_NULL;
/// The type of the C++ nullptr keyword.
typedef decltype(nullptr)	TYPE_OF_NULLPTR;

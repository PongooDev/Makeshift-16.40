// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

/**
* Windows specific types
**/
typedef char			ANSICHAR;
typedef wchar_t			WIDECHAR;
typedef uint8			UTF8CHAR;
typedef unsigned __int64	UPTRINT;
typedef __int64				PTRINT;

// Base defines, must define these for the platform, there are no defaults
#define PLATFORM_DESKTOP					1
#define PLATFORM_WINDOWS					1
#define PLATFORM_64BITS						1
#define PLATFORM_LITTLE_ENDIAN				1
#define PLATFORM_SUPPORTS_COLORIZED_OUTPUT_DEVICE	1

#define PLATFORM_SUPPORTS_PRAGMA_PACK						1
#define PLATFORM_ENABLE_VECTORINTRINSICS					1
#ifndef PLATFORM_MAYBE_HAS_SSE4_1 // May be set from UnrealBuildTool
	#define PLATFORM_MAYBE_HAS_SSE4_1						1
#endif
// Current unreal minspec is sse2, not sse4, so on windows any calling code must check _cpuid before calling SSE4 instructions;
// If called on a platform for which _cpuid for SSE4 returns false, attempting to call SSE4 intrinsics will crash
// If your title has raised the minspec to sse4, you can define PLATFORM_ALWAYS_HAS_SSE4_1 to 1
#ifndef PLATFORM_ALWAYS_HAS_SSE4_1 // May be set from UnrealBuildTool
	#define PLATFORM_ALWAYS_HAS_SSE4_1						0
#endif

// Function type macros.
#define VARARGS     __cdecl											/* Functions with variable arguments */
#undef CDECL
#define CDECL	    __cdecl											/* Standard C function */
#define STDCALL		__stdcall										/* Standard calling convention */
#define FORCEINLINE __forceinline									/* Force code to be inline */
#define FORCENOINLINE __declspec(noinline)							/* Force code to NOT be inline */

// Alignment.
#if defined(__clang__)
	#define GCC_PACK(n) __attribute__((packed,aligned(n)))
	#define GCC_ALIGN(n) __attribute__((aligned(n)))
	#if defined(_MSC_VER)
		#define MS_ALIGN(n) __declspec(align(n)) // With -fms-extensions, Clang will accept either alignment attribute
	#endif
#else
	#define MS_ALIGN(n) __declspec(align(n))
#endif

#if !defined(__clang__) || defined(_MSC_VER)
	#define ASSUME(expr) __assume(expr)
#endif

#define DECLARE_UINT64(x)	x

// Optimization macros (uses __pragma to enable inside a #define).
#if !defined(__clang__)
	#define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL __pragma(optimize("",off))
	#define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL  __pragma(optimize("",on))
#endif

// Backwater of the spec. All compilers support this except microsoft, and they will soon
#if !defined(__clang__)		// Clang expects typename outside template
	#define TYPENAME_OUTSIDE_TEMPLATE
#endif

// Tells the compiler to put the decorated function in a certain section (aka. segment) of the executable.
#define PLATFORM_CODE_SECTION(Name) __declspec(code_seg(Name))

// Strings.
#define LINE_TERMINATOR TEXT("\r\n")
#define LINE_TERMINATOR_ANSI "\r\n"

// Q: Why is there a __nop() before __debugbreak()?
// A: VS' debug engine has a bug where it will silently swallow explicit
// breakpoint interrupts when single-step debugging either line-by-line or
// over call instructions. This can hide legitimate reasons to trap. Asserts
// for example, which can appear as if the did not fire, leaving a programmer
// unknowingly debugging an undefined process.
#define PLATFORM_BREAK() (__nop(), __debugbreak())

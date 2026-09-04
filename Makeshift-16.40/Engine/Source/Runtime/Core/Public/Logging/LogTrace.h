// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

#if !defined(LOGTRACE_ENABLED)
#if UE_TRACE_ENABLED && !UE_BUILD_SHIPPING
#define LOGTRACE_ENABLED 1
#else
#define LOGTRACE_ENABLED 0
#endif
#endif

#if LOGTRACE_ENABLED

#error "Makeshift: log tracing (Unreal Insights) is not available in the DLL; the shipping game has it compiled out."

#else
#define TRACE_LOG_CATEGORY(Category, Name, DefaultVerbosity)
#define TRACE_LOG_MESSAGE(Category, Verbosity, Format, ...)
#endif

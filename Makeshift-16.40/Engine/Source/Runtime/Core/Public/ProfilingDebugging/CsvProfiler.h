// Copyright Epic Games, Inc. All Rights Reserved.

/**
*
* A lightweight multi-threaded CSV profiler which can be used for profiling in Test/Shipping builds
*/

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

// Whether to allow the CSV profiler in shipping builds.
// Enable in a .Target.cs file if required.
#ifndef CSV_PROFILER_ENABLE_IN_SHIPPING
#define CSV_PROFILER_ENABLE_IN_SHIPPING 0
#endif

// Enables command line switches and unit tests of the CSV profiler.
// The default disables these features in a shipping build, but a .Target.cs file can override this.
#ifndef CSV_PROFILER_ALLOW_DEBUG_FEATURES
#define CSV_PROFILER_ALLOW_DEBUG_FEATURES (!UE_BUILD_SHIPPING)
#endif

#ifndef CSV_PROFILER_USE_CUSTOM_FRAME_TIMINGS
#define CSV_PROFILER_USE_CUSTOM_FRAME_TIMINGS 0
#endif

#define CSV_PROFILER 0

#if CSV_PROFILER

#else
  #define CSV_CATEGORY_INDEX(CategoryName)
  #define CSV_CATEGORY_INDEX_GLOBAL
  #define CSV_STAT_FNAME(StatName)
  #define CSV_SCOPED_TIMING_STAT(Category,StatName)
  #define CSV_SCOPED_TIMING_STAT_GLOBAL(StatName)
  #define CSV_SCOPED_TIMING_STAT_EXCLUSIVE(StatName)
  #define CSV_SCOPED_TIMING_STAT_EXCLUSIVE_CONDITIONAL(StatName,Condition)
  #define CSV_SCOPED_WAIT_CONDITIONAL(Condition)
  #define CSV_SCOPED_SET_WAIT_STAT(StatName)
  #define CSV_SCOPED_SET_WAIT_STAT_IGNORE()
  #define CSV_CUSTOM_STAT(Category,StatName,Value,Op)
  #define CSV_CUSTOM_STAT_GLOBAL(StatName,Value,Op)
  #define CSV_DEFINE_STAT(Category,StatName)
  #define CSV_DEFINE_STAT_GLOBAL(StatName)
  #define CSV_DECLARE_STAT_EXTERN(Category,StatName)
  #define CSV_CUSTOM_STAT_DEFINED(StatName,Value,Op)
  #define CSV_DEFINE_CATEGORY(CategoryName,bDefaultValue)
  #define CSV_DECLARE_CATEGORY_EXTERN(CategoryName)
  #define CSV_DEFINE_CATEGORY_MODULE(Module_API,CategoryName,bDefaultValue)
  #define CSV_DECLARE_CATEGORY_MODULE_EXTERN(Module_API,CategoryName)
  #define CSV_EVENT(Category, Format, ...)
  #define CSV_EVENT_GLOBAL(Format, ...)
  #define CSV_METADATA(Key,Value)
#endif

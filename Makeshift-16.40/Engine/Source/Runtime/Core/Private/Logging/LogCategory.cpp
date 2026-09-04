// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogCategory.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDeviceRedirector.h"

FLogCategoryBase::FLogCategoryBase(const FLogCategoryName& InCategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity)
	: DefaultVerbosity(InDefaultVerbosity)
	, CompileTimeVerbosity(InCompileTimeVerbosity)
	, CategoryName(InCategoryName)
{
	constexpr uintptr_t FLogCategoryBase_Ctor = 0x1747844;

	void (*Fn)(FLogCategoryBase*, const FLogCategoryName*, ELogVerbosity::Type, ELogVerbosity::Type) = decltype(Fn)(InSDKUtils::GetImageBase() + FLogCategoryBase_Ctor);
	Fn(this, &InCategoryName, InDefaultVerbosity, InCompileTimeVerbosity);

	checkSlow(!(Verbosity & ELogVerbosity::BreakOnLog)); // this bit is factored out of this variable, always
}

FLogCategoryBase::~FLogCategoryBase()
{
	checkSlow(!(Verbosity & ELogVerbosity::BreakOnLog)); // this bit is factored out of this variable, always

	constexpr uintptr_t FLogCategoryBase_Dtor = 0x17478A0;

	void (*Fn)(FLogCategoryBase*) = decltype(Fn)(InSDKUtils::GetImageBase() + FLogCategoryBase_Dtor);
	Fn(this);
}

void FLogCategoryBase::SetVerbosity(ELogVerbosity::Type NewVerbosity)
{
	// regularize the verbosity to be at most whatever we were compiled with
	const ELogVerbosity::Type MaskedVerbosity = (ELogVerbosity::Type)(NewVerbosity & ELogVerbosity::VerbosityMask);
	Verbosity = CompileTimeVerbosity < MaskedVerbosity ? CompileTimeVerbosity : MaskedVerbosity;
	DebugBreakOnLog = !!(NewVerbosity & ELogVerbosity::BreakOnLog);
	checkSlow(!(Verbosity & ELogVerbosity::BreakOnLog)); // this bit is factored out of this variable, always
}

void FLogCategoryBase::ResetFromDefault()
{
	// regularize the default verbosity to be at most whatever we were compiled with
	SetVerbosity(ELogVerbosity::Type(DefaultVerbosity));
}

void FLogCategoryBase::PostTrigger(ELogVerbosity::Type VerbosityLevel)
{
	checkSlow(!(Verbosity & ELogVerbosity::BreakOnLog)); // this bit is factored out of this variable, always
	check(VerbosityLevel <= CompileTimeVerbosity); // we should have never gotten here, the compile-time version should ALWAYS be checked first
	if (DebugBreakOnLog || (VerbosityLevel & ELogVerbosity::BreakOnLog))  // we break if either the suppression level on this message is set to break or this log statement is set to break
	{
		GLog->FlushThreadedLogs();
		DebugBreakOnLog = false; // toggle this off automatically
		UE_DEBUG_BREAK();
	}
}

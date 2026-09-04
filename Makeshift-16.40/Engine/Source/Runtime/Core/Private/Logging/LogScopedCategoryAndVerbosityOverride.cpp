// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogScopedCategoryAndVerbosityOverride.h"

FLogScopedCategoryAndVerbosityOverride::FLogScopedCategoryAndVerbosityOverride(FName Category, ELogVerbosity::Type Verbosity)
{
	FOverride* TLS = GetTLSCurrent();
	Backup = *TLS;
	*TLS = FOverride(Category, Verbosity);
}

FLogScopedCategoryAndVerbosityOverride::~FLogScopedCategoryAndVerbosityOverride()
{
	FOverride* TLS = GetTLSCurrent();
	*TLS = Backup;
}

FLogScopedCategoryAndVerbosityOverride::FOverride* FLogScopedCategoryAndVerbosityOverride::GetTLSCurrent()
{
	constexpr uintptr_t GetTLSCurrent_Offset = 0x149FFAC;

	FOverride* (*Fn)() = decltype(Fn)(ImageBase + GetTLSCurrent_Offset);
	return Fn();
}

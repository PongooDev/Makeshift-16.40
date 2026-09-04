// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDeviceRedirector.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"

/*-----------------------------------------------------------------------------
	FOutputDeviceRedirector.
-----------------------------------------------------------------------------*/

FOutputDeviceRedirector* FOutputDeviceRedirector::Get()
{
	constexpr uintptr_t Get_Offset = 0x173AB48;

	FOutputDeviceRedirector* (*Fn)() = decltype(Fn)(ImageBase + Get_Offset);
	return Fn();
}

template<class T>
void FOutputDeviceRedirector::SerializeImpl(const TCHAR* Data, ELogVerbosity::Type Verbosity, T& Category, const double Time)
{
	constexpr uintptr_t SerializeImpl_Offset = 0xE8F06C;

	void (*Fn)(FOutputDeviceRedirector*, const TCHAR*, ELogVerbosity::Type, const FName*, double) = decltype(Fn)(ImageBase + SerializeImpl_Offset);
	Fn(this, Data, Verbosity, &Category, Time);
}

void FOutputDeviceRedirector::Serialize( const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category, const double Time )
{
	SerializeImpl(Data, Verbosity, Category, Time);
}

void FOutputDeviceRedirector::Serialize( const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category )
{
	SerializeImpl( Data, Verbosity, Category, -1.0 );
}

void FOutputDeviceRedirector::RedirectLog(const FName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Data)
{
	SerializeImpl(Data, Verbosity, Category, -1.0);
}

CORE_API FOutputDeviceRedirector* GetGlobalLogSingleton()
{
	return FOutputDeviceRedirector::Get();
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CommandLine.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"

/*-----------------------------------------------------------------------------
	FCommandLine
-----------------------------------------------------------------------------*/

bool& FCommandLine::bIsInitialized = *reinterpret_cast<bool*>(InSDKUtils::GetImageBase() + 0x9386E00);
TCHAR (&FCommandLine::CmdLine)[FCommandLine::MaxCommandLineSize] = *reinterpret_cast<TCHAR(*)[FCommandLine::MaxCommandLineSize]>(InSDKUtils::GetImageBase() + 0x937EE00);
TCHAR (&FCommandLine::OriginalCmdLine)[FCommandLine::MaxCommandLineSize] = *reinterpret_cast<TCHAR(*)[FCommandLine::MaxCommandLineSize]>(InSDKUtils::GetImageBase() + 0x9376E00);
TCHAR (&FCommandLine::LoggingCmdLine)[FCommandLine::MaxCommandLineSize] = *reinterpret_cast<TCHAR(*)[FCommandLine::MaxCommandLineSize]>(InSDKUtils::GetImageBase() + 0x936EE00);
TCHAR (&FCommandLine::LoggingOriginalCmdLine)[FCommandLine::MaxCommandLineSize] = *reinterpret_cast<TCHAR(*)[FCommandLine::MaxCommandLineSize]>(InSDKUtils::GetImageBase() + 0x9366E00);

bool FCommandLine::IsInitialized()
{
	return bIsInitialized;
}

const TCHAR* FCommandLine::Get()
{
	UE_CLOG(!bIsInitialized, LogInit, Fatal, TEXT("Attempting to get the command line but it hasn't been initialized yet."));
	return CmdLine;
}

const TCHAR* FCommandLine::GetForLogging()
{
	UE_CLOG(!bIsInitialized, LogInit, Fatal, TEXT("Attempting to get the command line but it hasn't been initialized yet."));
	return LoggingCmdLine;
}

const TCHAR* FCommandLine::GetOriginal()
{
	UE_CLOG(!bIsInitialized, LogInit, Fatal, TEXT("Attempting to get the command line but it hasn't been initialized yet."));
	return OriginalCmdLine;
}

const TCHAR* FCommandLine::GetOriginalForLogging()
{
	UE_CLOG(!bIsInitialized, LogInit, Fatal, TEXT("Attempting to get the command line but it hasn't been initialized yet."));
	return LoggingOriginalCmdLine;
}

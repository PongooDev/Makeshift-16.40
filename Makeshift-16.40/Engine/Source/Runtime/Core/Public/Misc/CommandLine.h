// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

struct CORE_API FCommandLine
{
	/** maximum size of the command line */
	static const uint32 MaxCommandLineSize = 16384;

	/**
	 * Returns an edited version of the executable's command line with the game name and certain other parameters removed.
	 */
	static const TCHAR* Get();

	/**
	 * Returns an edited version of the executable's command line.
	 */
	static const TCHAR* GetForLogging();

	/**
	 * Returns the command line originally passed to the executable.
	 */
	static const TCHAR* GetOriginal();

	/**
	 * Returns an edited version of the command line originally passed to the executable.
	 */
	static const TCHAR* GetOriginalForLogging();

	/**
	 * Checks if the command line has been initialized.
	 */
	static bool IsInitialized();

private:
	/** Flag to check if the commandline has been initialized or not. */
	static bool& bIsInitialized;
	/** character buffer containing the command line */
	static TCHAR (&CmdLine)[MaxCommandLineSize];
	/** character buffer containing the original command line */
	static TCHAR (&OriginalCmdLine)[MaxCommandLineSize];
	/** character buffer containing the command line filtered for logging purposes */
	static TCHAR (&LoggingCmdLine)[MaxCommandLineSize];
	/** character buffer containing the original command line filtered for logging purposes */
	static TCHAR (&LoggingOriginalCmdLine)[MaxCommandLineSize];
};

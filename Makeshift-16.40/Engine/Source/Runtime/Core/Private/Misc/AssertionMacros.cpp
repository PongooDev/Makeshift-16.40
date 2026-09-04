// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/Misc/AssertionMacros.h"
#include "Engine/Source/Runtime/Core/Public/Misc/VarArgs.h"
#include "Engine/Source/Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Engine/Source/Runtime/Core/Public/Templates/UnrealTemplate.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CString.h"
#include "Engine/Source/Runtime/Core/Public/Containers/StringConv.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDeviceRedirector.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDeviceError.h"

#include <atomic>

namespace
{
	// used to track state of assets/ensures
	bool& GetHasAsserted()
	{
		constexpr uintptr_t bHasAsserted_Offset = 0x946EF79;
		return *reinterpret_cast<bool*>(ImageBase + bHasAsserted_Offset);
	}
	std::atomic<SIZE_T> NumEnsureFailures {0};
	volatile long ActiveEnsureCount = 0;
}

#define FILE_LINE_DESC TEXT(" [File:%s] [Line: %i] ")

/*
	Ensure behavior

	* ensure() macro calls OptionallyLogFormattedEnsureMessageReturningFalse
	* OptionallyLogFormattedEnsureMessageReturningFalse calls EnsureFailed()
	* EnsureFailed() -
		* Formats the ensure failure and calls StaticFailDebug to populate the global error info (without callstack)
		* Prints the script callstack (if any)
		* Halts if a debugger is attached
		* If not, logs the callstack and attempts to submit an error report
	* execution continues as normal, (on some platforms this can take ~30 secs to perform)

	Check behavior

	* check() macro calls LogAssertFailedMessage
	* LogAssertFailedMessage formats the assertion message and calls StaticFailDebug
	* StaticFailDebug populates global error info with the failure message and if supported (AllowsCallStackDumpDuringAssert) the callstack
	* If a debugger is attached execution halts
	* If not FDebug::AssertFailed is called
	* FDebug::AssertFailed logs the assert message and description to GError
	* At this point behavior depends on the platform-specific error output device implementation
		* Desktop platforms (Windows, Mac, Linux) will generally throw an exception and in the handler attempt to submit a crash report and exit
		* Console platforms will generally dump the info to the log and abort()

	Fatal-error behavior

	* The UE_LOG macro calls FMsg::Logf which checks for "Fatal" verbosity
	* FMsg::Logf formats the failure message and calls StaticFailDebug
	* StaticFailDebug populates global error info with the failure message and if supported (AllowsCallStackDumpDuringAssert) the callstack
	* FDebug::AssertFailed is then called, and from this point behavior is identical to an assert but with a different message
*/

static void AssertFailedImplV(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Format, va_list Args)
{
	constexpr uintptr_t AssertFailedImplV_Offset = 0x52411E4;

	void (*Fn)(const ANSICHAR*, const ANSICHAR*, int32, const TCHAR*, va_list) = decltype(Fn)(ImageBase + AssertFailedImplV_Offset);
	Fn(Expr, File, Line, Format, Args);
}

/**
 *	Prints error to the debug output,
 *	prompts for the remote debugging if there is not debugger, breaks into the debugger
 *	and copies the error into the global error message.
 */
FORCENOINLINE void StaticFailDebug( const TCHAR* Error, const ANSICHAR* File, int32 Line, const TCHAR* Description, bool bIsEnsure, int NumStackFramesToIgnore )
{
	constexpr uintptr_t StaticFailDebug_Offset = 0x524729C;

	void (*Fn)(const TCHAR*, const ANSICHAR*, int32, const TCHAR*) = decltype(Fn)(ImageBase + StaticFailDebug_Offset);
	Fn(Error, File, Line, Description);
}

/// track thread asserts
bool FDebug::HasAsserted()
{
	return GetHasAsserted();
}

// track ensures
bool FDebug::IsEnsuring()
{
	return ActiveEnsureCount > 0;
}
SIZE_T FDebug::GetNumEnsureFailures()
{
	return NumEnsureFailures.load();
}

#if DO_CHECK || DO_GUARD_SLOW || DO_ENSURE
// Failed assertion handler.
//warning: May be called at library startup time.

FORCENOINLINE void FDebug::LogAssertFailedMessageImpl(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Fmt, ...)
{
	va_list Args;
	va_start(Args, Fmt);
	LogAssertFailedMessageImplV(Expr, File, Line, Fmt, Args);
	va_end(Args);
}

void FDebug::LogAssertFailedMessageImplV(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Fmt, va_list Args)
{
	// Ignore this assert if we're already forcibly shutting down because of a critical error.
	if( !GIsCriticalError )
	{
		TCHAR DescriptionString[4096];
		FCString::GetVarArgs( DescriptionString, UE_ARRAY_COUNT(DescriptionString), Fmt, Args );

		TCHAR ErrorString[MAX_SPRINTF];
		FCString::Sprintf( ErrorString, TEXT( "Assertion failed: %s" ), ANSI_TO_TCHAR( Expr ) );

		const int32 NumStackFramesToIgnore = 1;
		StaticFailDebug( ErrorString, File, Line, DescriptionString, false, NumStackFramesToIgnore );
	}
}

/**
 * Called when an 'ensure' assertion fails; gathers stack data and generates and error report.
 *
 * @param	Expr	Code expression ANSI string (#code)
 * @param	File	File name ANSI string (__FILE__)
 * @param	Line	Line number (__LINE__)
 * @param	Msg		Informative error message text
 * @param	NumStackFramesToIgnore	Number of stack frames to ignore in the callstack
 */
FORCENOINLINE void FDebug::EnsureFailed(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Msg, int NumStackFramesToIgnore)
{
	++NumEnsureFailures;

	// You can set bShouldCrash to true to cause a regular assertion to trigger (stopping program execution) when an ensure() error occurs
	const bool bShouldCrash = false;		// By default, don't crash on ensure()
	if( bShouldCrash )
	{
		// Just trigger a regular assertion which will crash via GError->Logf()
		FDebug::LogAssertFailedMessageImpl( Expr, File, Line, TEXT("%s"), Msg );
		return;
	}

	// Should we spin here?
	InterlockedIncrement(&ActiveEnsureCount);

	// Print initial debug message for this error
	TCHAR ErrorString[MAX_SPRINTF];
	FCString::Sprintf(ErrorString,TEXT("Ensure condition failed: %s"),ANSI_TO_TCHAR(Expr));

	StaticFailDebug( ErrorString, File, Line, Msg, true, NumStackFramesToIgnore + 1 );

#if !NO_LOGGING
	UE_LOG(LogOutputDevice, Error, TEXT("%s") FILE_LINE_DESC TEXT("\n%s\n"), ErrorString, ANSI_TO_TCHAR(File), Line, Msg);
#endif
	GLog->Flush();

	InterlockedDecrement(&ActiveEnsureCount);
}

void FORCENOINLINE FDebug::CheckVerifyFailedImpl(
	const ANSICHAR* Expr,
	const ANSICHAR* File,
	const int Line,
	const TCHAR* Format,
	...)
{
	va_list Args;

	va_start(Args, Format);
	FDebug::LogAssertFailedMessageImplV(Expr, File, Line, Format, Args);
	va_end(Args);

	if (!FPlatformMisc::IsDebuggerPresent())
	{
		FPlatformMisc::PromptForRemoteDebugging(false);

		va_start(Args, Format);
		AssertFailedImplV(Expr, File, Line, Format, Args);
		va_end(Args);
	}
}

#endif // DO_CHECK || DO_GUARD_SLOW || DO_ENSURE

void VARARGS FDebug::AssertFailed(const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* Format/* = TEXT("")*/, ...)
{
	va_list Args;
	va_start(Args, Format);
	AssertFailedImplV(Expr, File, Line, Format, Args);
	va_end(Args);
}

void FDebug::ProcessFatalError()
{
	// This is not perfect because another thread might crash and be handled before this assert
	// but this static variable will report the crash as an assert. Given complexity of a thread
	// aware solution, this should be good enough. If crash reports are obviously wrong we can
	// look into fixing this.
	GetHasAsserted() = true;

	GError->Logf(TEXT("%s"), GErrorHist);
}

#if DO_CHECK || DO_GUARD_SLOW || DO_ENSURE
FORCENOINLINE bool VARARGS FDebug::OptionallyLogFormattedEnsureMessageReturningFalseImpl( bool bLog, const ANSICHAR* Expr, const ANSICHAR* File, int32 Line, const TCHAR* FormattedMsg, ... )
{
	if (bLog)
	{
		const int32 TempStrSize = 4096;
		TCHAR TempStr[ TempStrSize ];
		GET_VARARGS( TempStr, TempStrSize, TempStrSize - 1, FormattedMsg, FormattedMsg );

		const int32 NumStackFramesToIgnore = 1; // Just ignore this frame
		EnsureFailed( Expr, File, Line, TempStr, NumStackFramesToIgnore );
	}

	return false;
}
#endif

FORCENOINLINE void VARARGS LowLevelFatalErrorHandler(const ANSICHAR* File, int32 Line, const TCHAR* Format, ...)
{
	TCHAR DescriptionString[4096];
	GET_VARARGS( DescriptionString, UE_ARRAY_COUNT(DescriptionString), UE_ARRAY_COUNT(DescriptionString)-1, Format, Format );

	const int32 NumStackFramesToIgnore = 1; // Just ignore this frame
	StaticFailDebug(TEXT("LowLevelFatalError"), File, Line, DescriptionString, false, NumStackFramesToIgnore);
}

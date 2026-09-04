// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"
#include "Engine/Source/Runtime/Core/Public/Misc/VarArgs.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CString.h"

/*-----------------------------------------------------------------------------
	NAN Diagnostic Failure
-----------------------------------------------------------------------------*/

int32 GEnsureOnNANDiagnostic = false;

#if DO_CHECK
namespace UE4Asserts_Private
{
	void VARARGS InternalLogNANDiagnosticMessage(const TCHAR* FormattedMsg, ...)
	{
		const int32 TempStrSize = 4096;
		TCHAR TempStr[TempStrSize];
		GET_VARARGS(TempStr, TempStrSize, TempStrSize - 1, FormattedMsg, FormattedMsg);
		UE_LOG(LogCore, Error, TempStr);
	}
}
#endif

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

/*-----------------------------------------------------------------------------
	Parsing functions.
-----------------------------------------------------------------------------*/
struct CORE_API FParse
{
	/** Checks if a command-line parameter exists in the stream. */
	static bool Param( const TCHAR* Stream, const TCHAR* Param );
};

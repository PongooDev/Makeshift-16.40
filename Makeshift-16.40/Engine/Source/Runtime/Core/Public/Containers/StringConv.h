// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

#include <string>

class FANSIToTCHAR
{
public:
	explicit FANSIToTCHAR(const ANSICHAR* Source)
		: bIsNull(Source == nullptr)
	{
		if (Source)
		{
			Converted.reserve(::strlen(Source));
			for (const ANSICHAR* Ch = Source; *Ch; ++Ch)
			{
				Converted.push_back((unsigned char)*Ch <= 0x7F ? (TCHAR)*Ch : TEXT('?'));
			}
		}
	}

	FORCEINLINE const TCHAR* Get() const { return bIsNull ? nullptr : Converted.c_str(); }
	FORCEINLINE int32 Length() const { return (int32)Converted.length(); }

private:
	std::wstring Converted;
	bool bIsNull;
};

class FTCHARToANSI
{
public:
	explicit FTCHARToANSI(const TCHAR* Source)
		: bIsNull(Source == nullptr)
	{
		if (Source)
		{
			Converted.reserve(::wcslen(Source));
			for (const TCHAR* Ch = Source; *Ch; ++Ch)
			{
				Converted.push_back(*Ch <= 0x7F ? (ANSICHAR)*Ch : '?');
			}
		}
	}

	FORCEINLINE const ANSICHAR* Get() const { return bIsNull ? nullptr : Converted.c_str(); }
	FORCEINLINE int32 Length() const { return (int32)Converted.length(); }

private:
	std::string Converted;
	bool bIsNull;
};

#define TCHAR_TO_ANSI(str) (ANSICHAR*)FTCHARToANSI(static_cast<const TCHAR*>(str)).Get()
#define ANSI_TO_TCHAR(str) (TCHAR*)FANSIToTCHAR(static_cast<const ANSICHAR*>(str)).Get()

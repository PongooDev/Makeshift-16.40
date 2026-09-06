// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	ScriptCore.cpp: Kismet VM execution and support code.
=============================================================================*/

#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Script.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/CoreNative.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

FNativeFuncPtr (&GNatives)[EX_Max] = *reinterpret_cast<FNativeFuncPtr (*)[EX_Max]>(InSDKUtils::GetImageBase() + 0x938F760);

void FFrame::Step(UObject* Context, RESULT_DECL)
{
	int32 B = *Code++;
	(GNatives[B])(Context,*this,RESULT_PARAM);
}

void FFrame::StepExplicitProperty(void*const Result, FProperty* Property)
{
	void (*Fn)(FFrame*, void*const, FProperty*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0xC4E1EC);
	Fn(this, Result, Property);
}

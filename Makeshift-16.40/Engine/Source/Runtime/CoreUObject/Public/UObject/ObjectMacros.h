// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	ObjectMacros.h: Helper macros and defines for UObject system
=============================================================================*/

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/EnumClassFlags.h"

/** Objects flags for internal use (GC, low level UObject code) */
enum class EInternalObjectFlags : int32
{
	None = 0,
	//~ All the other bits are reserved, DO NOT ADD NEW FLAGS HERE!

	ReachableInCluster = 1 << 23, ///< External reference to object in cluster exists
	ClusterRoot = 1 << 24, ///< Root of a cluster
	Native = 1 << 25, ///< Native (UClass only).
	Async = 1 << 26, ///< Object exists only on a different thread than the game thread.
	AsyncLoading = 1 << 27, ///< Object is being asynchronously loaded.
	Unreachable = 1 << 28, ///< Object is not reachable on the object graph.
	PendingKill = 1 << 29, ///< Objects that are pending destruction (invalid for gameplay but valid objects)
	RootSet = 1 << 30, ///< Object will not be garbage collected, even if unreferenced.
	//~ UnusedFlag = 1 << 31,

	GarbageCollectionKeepFlags = Native | Async | AsyncLoading,

	//~ Make sure this is up to date!
	AllFlags = ReachableInCluster | ClusterRoot | Native | Async | AsyncLoading | Unreachable | PendingKill | RootSet
};
ENUM_CLASS_FLAGS(EInternalObjectFlags);

///////////////////////////////
/// UObject definition macros
///////////////////////////////

// These macros wrap metadata parsed by the Unreal Header Tool, and are otherwise
// ignored when code containing them is compiled by the C++ compiler
#define UPROPERTY(...)
#define UFUNCTION(...)
#define USTRUCT(...)
#define UMETA(...)
#define UPARAM(...)
#define UENUM(...)
#define UDELEGATE(...)
#define RIGVM_METHOD(...)

// Copyright Epic Games, Inc. All Rights Reserved.

//
// Base class of a network driver attached to an active or pending level.
//

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/NetworkGuid.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/CoreNetTypes.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetworkObjectList.h"

struct ENGINE_API FActorDestructionInfo
{
public:
	FActorDestructionInfo()
		: Reason(EChannelCloseReason::Destroyed)
		, bIgnoreDistanceCulling(false)
	{}

	TWeakObjectPtr<ULevel>		Level;
	TWeakObjectPtr<UObject>		ObjOuter;
	FVector			DestroyedPosition;
	FNetworkGUID	NetGUID;
	FString			PathName;
	FName			StreamingLevelName;
	EChannelCloseReason Reason;

	/** When true the destruction info data will be sent even if the viewers are not close to the actor */
	bool bIgnoreDistanceCulling;

};

//
// Priority sortable list.
//
struct FActorPriority
{
	int32						Priority;	// Update priority, higher = more important.

	FNetworkObjectInfo*			ActorInfo;	// Actor info.
	UActorChannel*		Channel;	// Actor channel.

	FActorDestructionInfo *	DestructionInfo;	// Destroy an actor

	FActorPriority() :
		Priority(0), ActorInfo(NULL), Channel(NULL), DestructionInfo(NULL)
	{}

	FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const TArray<FNetViewer>& Viewers, bool bLowBandwidth);
	FActorPriority(UNetConnection* InConnection, FActorDestructionInfo * DestructInfo, const TArray<FNetViewer>& Viewers );
};

struct FCompareFActorPriority
{
	FORCEINLINE bool operator()( const FActorPriority& A, const FActorPriority& B ) const
	{
		return B.Priority < A.Priority;
	}
};

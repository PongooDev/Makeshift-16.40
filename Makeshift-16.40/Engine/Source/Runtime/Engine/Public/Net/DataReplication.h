// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	DataReplication.h:
	Holds classes for data replication (properties and RPCs).
=============================================================================*/
#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/NetworkGuid.h"
#include "Engine/Source/Runtime/Core/Public/Templates/SharedPointer.h"
#include "Engine/Source/Runtime/Core/Public/Templates/UniquePtr.h"

class FNetFieldExportGroup;
class FOutBunch;
class FRepLayout;
class FRepState;

/**
 * Represents an object that is currently being replicated or handling RPCs.
 *
 *
 *
 *		|----------------|
 *		| NetGUID ObjRef |
 * 		|----------------|
 *      |                |
 *		| Properties...  |
 *		|                |
 *		| RPCs...        |
 *      |                |
 *      |----------------|
 *		| </End Tag>     |
 *		|----------------|
 *
 */
class ENGINE_API FObjectReplicator
{
public:

	FObjectReplicator();
	~FObjectReplicator();

	/** Takes Data, and compares against shadow state to log differences */
	bool ValidateAgainstState(const UObject* ObjectState);

public:

	/** Net GUID for the object we're replicating. */
	FNetworkGUID ObjectNetGUID;

	/** The amount of memory (in bytes) that we're using to track Unmapped GUIDs. */
	int32 TrackedGuidMemoryBytes;

	/** True if last update (ReplicateActor) produced no replicated properties */
	uint32 bLastUpdateEmpty : 1;

	/** Whether or not the Actor Channel on which we're replicating has been Opened / Acked by the receiver. */
	uint32 bOpenAckCalled : 1;

	/** True if we need to do an unmapped check next frame. */
	uint32 bForceUpdateUnmapped : 1;

	/** Whether or not we've already replicated properties this frame. */
	uint32 bHasReplicatedProperties : 1;

private:

	/** Whether or not we are going to use Fast Array Delta Struct Delta Serialization. See FFastArraySerializer::FastArrayDeltaSerialize_DeltaSerializeStructs. */
	uint32 bSupportsFastArrayDelta : 1;

public:

	TSharedPtr<class FReplicationChangelistMgr> ChangelistMgr;
	TSharedPtr<FRepLayout> RepLayout;
	TUniquePtr<FRepState>  RepState;
	TUniquePtr<FRepState> CheckpointRepState;

	UClass* ObjectClass;

	UObject* ObjectPtr;

	/** Connection this replicator was created on. */
	UNetConnection* Connection;

	/** The Actor Channel that we're replicating on. This expected to be owned by Connection. */
	UActorChannel* OwningChannel;
};

class ENGINE_API FScopedActorRoleSwap
{
public:
	FScopedActorRoleSwap(AActor* InActor);
	~FScopedActorRoleSwap();

	FScopedActorRoleSwap(const FScopedActorRoleSwap&) = delete;
	FScopedActorRoleSwap& operator=(const FScopedActorRoleSwap&) = delete;

	FScopedActorRoleSwap(FScopedActorRoleSwap&& Other)
	{
		Actor = Other.Actor;
		Other.Actor = nullptr;
	}
	FScopedActorRoleSwap& operator=(FScopedActorRoleSwap&& Other)
	{
		Actor = Other.Actor;
		Other.Actor = nullptr;
		return *this;
	}

private:
	AActor* Actor;
};

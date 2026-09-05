// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Templates/SharedPointer.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetConnection.h"

class FArchive;

/**
 * Struct to store an actor pointer and any internal metadata for that actor used
 * internally by a UNetDriver.
 */
struct FNetworkObjectInfo
{
	/** Pointer to the replicated actor. */
	AActor* Actor;

	/** WeakPtr to actor. This is cached here to prevent constantly constructing one when needed for (things like) keys in TMaps/TSets */
	TWeakObjectPtr<AActor> WeakActor;

	/** Next time to consider replicating the actor. Based on FPlatformTime::Seconds(). */
	double NextUpdateTime;

	/** Last absolute time in seconds since actor actually sent something during replication */
	double LastNetReplicateTime;

	/** Optimal delta between replication updates based on how frequently actor properties are actually changing */
	float OptimalNetUpdateDelta;

	/** Last time this actor was updated for replication via NextUpdateTime
	* @warning: internal net driver time, not related to WorldSettings.TimeSeconds */
	UE_DEPRECATED(4.25, "Please use LastNetUpdateTimestamp instead.")
	float LastNetUpdateTime;
	double LastNetUpdateTimestamp;

	/** List of connections that this actor is dormant on */
	TSet<TWeakObjectPtr<UNetConnection>> DormantConnections;

	/** A list of connections that this actor has recently been dormant on, but the actor doesn't have a channel open yet.
	*  These need to be differentiated from actors that the client doesn't know about, but there's no explicit list for just those actors.
	*  (this list will be very transient, with connections being moved off the DormantConnections list, onto this list, and then off once the actor has a channel again)
	*/
	TSet<TWeakObjectPtr<UNetConnection>> RecentlyDormantConnections;

	/** Is this object still pending a full net update due to clients that weren't able to replicate the actor at the time of LastNetUpdateTime */
	uint8 bPendingNetUpdate : 1;

	/** Force this object to be considered relevant for at least one update */
	UE_DEPRECATED(4.23, "Use the ForceRelevantFrame variable since this variable is not set anymore.")
	uint8 bForceRelevantNextUpdate : 1;

	/** Should this object be considered for replay checkpoint writes */
	uint8 bDirtyForReplay : 1;

	/** Should channel swap roles while calling ReplicateActor */
	uint8 bSwapRolesOnReplicate : 1;

	/** Force this object to be considered relevant for at least one update */
	uint32 ForceRelevantFrame = 0;

	FNetworkObjectInfo()
		: Actor(nullptr)
		, NextUpdateTime(0.0)
		, LastNetReplicateTime(0.0)
		, OptimalNetUpdateDelta(0.0f)
		, LastNetUpdateTime(0.0f)
		, LastNetUpdateTimestamp(0.0)
		, bPendingNetUpdate(false)
		, bForceRelevantNextUpdate(false)
		, bDirtyForReplay(false)
		, bSwapRolesOnReplicate(false) {}

	FNetworkObjectInfo(AActor* InActor)
		: Actor(InActor)
		, WeakActor(InActor)
		, NextUpdateTime(0.0)
		, LastNetReplicateTime(0.0)
		, OptimalNetUpdateDelta(0.0f)
		, LastNetUpdateTime(0.0f)
		, LastNetUpdateTimestamp(0.0)
		, bPendingNetUpdate(false)
		, bForceRelevantNextUpdate(false)
		, bDirtyForReplay(false)
		, bSwapRolesOnReplicate(false) {}

	void CountBytes(FArchive& Ar) const;
};

/**
 * KeyFuncs to allow using the actor pointer as the comparison key in a set.
 */
struct FNetworkObjectKeyFuncs : BaseKeyFuncs<TSharedPtr<FNetworkObjectInfo>, AActor*, false>
{
	/**
	 * @return The key used to index the given element.
	 */
	static KeyInitType GetSetKey(ElementInitType Element)
	{
		return Element.Get()->Actor;
	}

	/**
	 * @return True if the keys match.
	 */
	static bool Matches(KeyInitType A,KeyInitType B)
	{
		return A == B;
	}

	/** Calculates a hash index for a key. */
	static uint32 GetKeyHash(KeyInitType Key)
	{
		return GetTypeHash(Key);
	}
};

/**
 * Stores the list of replicated actors for a given UNetDriver.
 */
class ENGINE_API FNetworkObjectList
{
public:
	typedef TSet<TSharedPtr<FNetworkObjectInfo>, FNetworkObjectKeyFuncs> FNetworkObjectSet;

	/** Returns a const reference to the entire set of tracked actors. */
	const FNetworkObjectSet& GetAllObjects() const { return AllNetworkObjects; }

	/** Returns a const reference to the active set of tracked actors. */
	const FNetworkObjectSet& GetActiveObjects() const { return ActiveNetworkObjects; }

	/** Returns a const reference to the entire set of dormant actors. */
	const FNetworkObjectSet& GetDormantObjectsOnAllConnections() const { return ObjectsDormantOnAllConnections; }

	int32 GetNumDormantActorsForConnection( UNetConnection* const Connection ) const;

private:
	FNetworkObjectSet AllNetworkObjects;
	FNetworkObjectSet ActiveNetworkObjects;
	FNetworkObjectSet ObjectsDormantOnAllConnections;

	TMap<TWeakObjectPtr<UNetConnection>, int32 > NumDormantObjectsPerConnection;
};

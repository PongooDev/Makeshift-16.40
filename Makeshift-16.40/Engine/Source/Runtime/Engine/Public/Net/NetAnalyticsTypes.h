// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"

namespace SDK
{
	class UNetConnection;
}

/** Struct wrapping Per Net Connection saturation analytics. */
struct ENGINE_API FNetConnectionSaturationAnalytics
{
public:

	FNetConnectionSaturationAnalytics()
		: NumberOfTrackedFrames(0)
		, NumberOfSaturatedFrames(0)
		, LongestRunOfSaturatedFrames(0)
		, NumberOfReplications(0)
		, NumberOfSaturatedReplications(0)
		, LongestRunOfSaturatedReplications(0)
		, CurrentRunOfSaturatedFrames(0)
		, CurrentRunOfSaturatedReplications(0)
	{
	}

	/** The total number of frames that we have currently tracked. */
	const uint32 GetNumberOfTrackedFrames() const
	{
		return NumberOfTrackedFrames;
	}

	/** The number of frames we have reported as saturated.*/
	const uint32 GetNumberOfSaturatedFrames() const
	{
		return NumberOfSaturatedFrames;
	}

	/** The longest number of consecutive frames that we have been saturated. */
	const uint32 GetLongestRunOfSaturatedFrames() const
	{
		return LongestRunOfSaturatedFrames;
	}

	/**
	 * The number of times we have tried to replicate data on this connection
	 * (UNetDriver::ServerReplicateActors / UReplicationGraph::ServerReplicateActors)
	 */
	const uint32 GetNumberOfReplications() const
	{
		return NumberOfReplications;
	}

	/** The number of times we have been pre-empted from replicating all data, due to saturation. */
	const uint32 GetNumberOfSaturatedReplications() const
	{
		return NumberOfSaturatedReplications;
	}

	/** The longest number of consecutive replication attempts where we were pre-empted due to saturation. */
	const uint32 GetLongestRunOfSaturatedReplications() const
	{
		return LongestRunOfSaturatedReplications;
	}

	/** Resets the state of tracking. */
	void Reset();

private:

	friend class SDK::UNetConnection;

	void TrackFrame(const bool bIsSaturated);

	void TrackReplication(const bool bIsSaturated);

	uint32 NumberOfTrackedFrames;
	uint32 NumberOfSaturatedFrames;
	uint32 LongestRunOfSaturatedFrames;

	uint32 NumberOfReplications;
	uint32 NumberOfSaturatedReplications;
	uint32 LongestRunOfSaturatedReplications;

	uint32 CurrentRunOfSaturatedFrames;
	uint32 CurrentRunOfSaturatedReplications;
};

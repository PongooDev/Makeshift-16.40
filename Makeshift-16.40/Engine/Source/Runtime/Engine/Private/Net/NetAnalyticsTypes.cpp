// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Engine/Public/Net/NetAnalyticsTypes.h"

void FNetConnectionSaturationAnalytics::TrackFrame(const bool bIsSaturated)
{
	++NumberOfTrackedFrames;

	if (bIsSaturated)
	{
		++NumberOfSaturatedFrames;
		++CurrentRunOfSaturatedFrames;
        if (CurrentRunOfSaturatedFrames > LongestRunOfSaturatedFrames)
        {
            LongestRunOfSaturatedFrames = CurrentRunOfSaturatedFrames;
        }
	}
	else
	{
		CurrentRunOfSaturatedFrames = 0;
	}
}

void FNetConnectionSaturationAnalytics::TrackReplication(const bool bIsSaturated)
{
	++NumberOfReplications;

	if (bIsSaturated)
	{
		++NumberOfSaturatedReplications;
		++CurrentRunOfSaturatedReplications;
        if (CurrentRunOfSaturatedReplications > LongestRunOfSaturatedReplications)
        {
            LongestRunOfSaturatedReplications = CurrentRunOfSaturatedReplications;
        }
	}
	else
	{
		CurrentRunOfSaturatedReplications = 0;
	}
}

void FNetConnectionSaturationAnalytics::Reset()
{
	NumberOfSaturatedFrames = 0;
	NumberOfTrackedFrames = 0;
	LongestRunOfSaturatedFrames = 0;

	NumberOfReplications = 0;
	NumberOfSaturatedReplications = 0;
	LongestRunOfSaturatedReplications = 0;

	CurrentRunOfSaturatedFrames = 0;
	CurrentRunOfSaturatedReplications = 0;
}

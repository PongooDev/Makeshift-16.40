// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	NetConnection.cpp: Unreal connection base class.
=============================================================================*/

#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetConnection.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetDriver.h"

void UNetConnection::RemoveDestructionInfo(FActorDestructionInfo* DestructionInfo)
{
	if (ReplicationConnectionDriver)
	{
		ReplicationConnectionDriver->NotifyRemoveDestructionInfo(DestructionInfo);
	}
	else
	{
		DestroyedStartupOrDormantActorGUIDs.Remove(DestructionInfo->NetGUID);
	}
}

void UNetConnection::TrackReplicationForAnalytics(const bool bWasSaturated)
{
	++TickCount;
	SaturationAnalytics.TrackReplication(bWasSaturated);
}

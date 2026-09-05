// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	DataChannel.cpp: Unreal datachannel implementation.
=============================================================================*/

#include "pch.h"
#include "Engine/Source/Runtime/Engine/Public/Net/DataReplication.h"

bool FObjectReplicator::ValidateAgainstState( const UObject* ObjectState )
{
	constexpr uintptr_t Offset = 0x5EBEBA8;

	return reinterpret_cast<bool (*)(FObjectReplicator*, const UObject*)>(ImageBase + Offset)(this, ObjectState);
}

FScopedActorRoleSwap::FScopedActorRoleSwap(AActor* InActor)
	: Actor(InActor)
{
	const bool bShouldSwapRoles = Actor != nullptr && Actor->GetRemoteRole() == ROLE_Authority;

	if (bShouldSwapRoles)
	{
		Actor->SwapRoles();
	}
	else
	{
		Actor = nullptr;
	}
}

FScopedActorRoleSwap::~FScopedActorRoleSwap()
{
	if (Actor != nullptr)
	{
		Actor->SwapRoles();
	}
}

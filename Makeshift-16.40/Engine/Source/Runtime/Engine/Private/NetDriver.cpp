// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	NetworkDriver.cpp: Unreal network driver base class.
=============================================================================*/

#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetDriver.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetConnection.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Channel.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/ActorChannel.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetworkObjectList.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/PackageMapClient.h"
#include "Engine/Source/Runtime/Engine/Public/Net/DataReplication.h"
#include "Engine/Source/Runtime/Engine/Public/NetworkingDistanceConstants.h"
#include "Engine/Source/Runtime/Core/Public/Misc/MemStack.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CommandLine.h"
#include "Engine/Source/Runtime/Core/Public/Misc/Parse.h"
#include "Engine/Source/Runtime/Core/Public/HAL/IConsoleManager.h"
#include "Engine/Source/Runtime/Core/Public/Templates/Sorting.h"
#include "Engine/Source/Runtime/Core/Public/Math/UnrealMathUtility.h"
#include "Engine/Source/Runtime/Core/Public/Stats/Stats.h"
#include "Engine/Source/Runtime/Engine/Public/EngineStats.h"
#include "Engine/Source/Runtime/Core/Public/ProfilingDebugging/CsvProfiler.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectBaseUtility.h"

DEFINE_STAT(STAT_NumConsideredActors);
DEFINE_STAT(STAT_PrioritizedActors);
DEFINE_STAT(STAT_NumReplicatedActors);
DEFINE_STAT(STAT_NumReplicatedActorBytes);
DEFINE_STAT(STAT_NumRelevantDeletedActors);
DEFINE_STAT(STAT_NumInitiallyDormantActors);

DECLARE_CYCLE_STAT(TEXT("Process Prioritized Actors Time"), STAT_NetProcessPrioritizedActorsTime, STATGROUP_Game);

int32 GNumSaturatedConnections;

DECLARE_DWORD_COUNTER_STAT(TEXT("Num Saturated Connections"), STAT_NumSaturatedConnections, STATGROUP_Net);

int32& GSetNetDormancyEnabled = *reinterpret_cast<int32*>(InSDKUtils::GetImageBase() + 0x92CBF7C);

int32& GNetDormancyValidate = *reinterpret_cast<int32*>(InSDKUtils::GetImageBase() + 0x9514684);

static TAutoConsoleVariable<int32>& CVarUseAdaptiveNetUpdateFrequency = *reinterpret_cast<TAutoConsoleVariable<int32>*>(InSDKUtils::GetImageBase() + 0x95EA9D0);

FString UNetDriver::GetDescription()
{
	FString NetDriverNameString;
	NetDriverName.ToString(NetDriverNameString);

	return FString::Printf(TEXT("%s %s%s"), *NetDriverNameString, *GetNameSafe(this), bIsPeer ? TEXT("(PEER)") : TEXT(""));
}

/*static*/ bool UNetDriver::IsAdaptiveNetUpdateFrequencyEnabled()
{
	const bool bUseAdapativeNetFrequency = CVarUseAdaptiveNetUpdateFrequency.GetValueOnAnyThread() > 0;
	return bUseAdapativeNetFrequency;
}

FActorPriority::FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const TArray<FNetViewer>& Viewers, bool bLowBandwidth)
	: ActorInfo(InActorInfo), Channel(InChannel), DestructionInfo(NULL)
{
	const float Time = Channel ? (InConnection->Driver->GetElapsedTime() - Channel->LastUpdateTime) : InConnection->Driver->SpawnPrioritySeconds;
	// take the highest priority of the viewers on this connection
	Priority = 0;
	for (int32 i = 0; i < Viewers.Num(); i++)
	{
		Priority = FMath::Max<int32>(Priority, FMath::RoundToInt(65536.0f * ActorInfo->Actor->GetNetPriority(Viewers[i].ViewLocation, Viewers[i].ViewDir, Viewers[i].InViewer, Viewers[i].ViewTarget, InChannel, Time, bLowBandwidth)));
	}
}

FActorPriority::FActorPriority(UNetConnection* InConnection, FActorDestructionInfo * Info, const TArray<FNetViewer>& Viewers )
	: ActorInfo(NULL), Channel(NULL), DestructionInfo(Info)
{

	Priority = 0;

	for (int32 i = 0; i < Viewers.Num(); i++)
	{
		float Time  = InConnection->Driver->SpawnPrioritySeconds;

		FVector Dir = DestructionInfo->DestroyedPosition - Viewers[i].ViewLocation;
		float DistSq = Dir.SizeSquared();

		// adjust priority based on distance and whether actor is in front of viewer
		if ( (Viewers[i].ViewDir | Dir) < 0.f )
		{
			if ( DistSq > NEARSIGHTTHRESHOLDSQUARED )
				Time *= 0.2f;
			else if ( DistSq > CLOSEPROXIMITYSQUARED )
				Time *= 0.4f;
		}
		else if ( DistSq > MEDSIGHTTHRESHOLDSQUARED )
			Time *= 0.4f;

		Priority = FMath::Max<int32>(Priority, 65536.0f * Time);
	}
}

namespace NetCmds
{
	static FAutoConsoleVariable& MaxConnectionsToTickPerServerFrame = *reinterpret_cast<FAutoConsoleVariable*>(InSDKUtils::GetImageBase() + 0x95EAA78);
}

int32 UNetDriver::ServerReplicateActors_PrepConnections( const float DeltaSeconds )
{
	int32 NumClientsToTick = ClientConnections.Num();

	// by default only throttle update for listen servers unless specified on the commandline
	static bool bForceClientTickingThrottle = FParse::Param( FCommandLine::Get(), TEXT( "limitclientticks" ) );
	if ( bForceClientTickingThrottle || GetNetMode() == NM_ListenServer )
	{
		// determine how many clients to tick this frame based on GEngine->NetTickRate (always tick at least one client), double for lan play
		// FIXME: DeltaTimeOverflow is a static, and will conflict with other running net drivers, we investigate storing it on the driver itself!
		static float DeltaTimeOverflow = 0.f;
		// updates are doubled for lan play
		static bool LanPlay = FParse::Param( FCommandLine::Get(), TEXT( "lanplay" ) );
		//@todo - ideally we wouldn't want to tick more clients with a higher deltatime as that's not going to be good for performance and probably saturate bandwidth in hitchy situations, maybe
		// come up with a solution that is greedier with higher framerates, but still won't risk saturating server upstream bandwidth
		float ClientUpdatesThisFrame = GEngine->NetClientTicksPerSecond * ( DeltaSeconds + DeltaTimeOverflow ) * ( LanPlay ? 2.f : 1.f );
		NumClientsToTick = FMath::Min<int32>( NumClientsToTick, FMath::TruncToInt( ClientUpdatesThisFrame ) );
		//UE_LOG(LogNet, Log, TEXT("%2.3f: Ticking %d clients this frame, %2.3f/%2.4f"),GetWorld()->GetTimeSeconds(),NumClientsToTick,DeltaSeconds,ClientUpdatesThisFrame);
		if ( NumClientsToTick == 0 )
		{
			// if no clients are ticked this frame accumulate the time elapsed for the next frame
			DeltaTimeOverflow += DeltaSeconds;
			return 0;
		}
		DeltaTimeOverflow = 0.f;
	}

	if( NetCmds::MaxConnectionsToTickPerServerFrame->GetInt() > 0 )
	{
		NumClientsToTick = FMath::Min( ClientConnections.Num(), NetCmds::MaxConnectionsToTickPerServerFrame->GetInt() );
	}

	bool bFoundReadyConnection = false;

	for ( int32 ConnIdx = 0; ConnIdx < ClientConnections.Num(); ConnIdx++ )
	{
		UNetConnection* Connection = ClientConnections[ConnIdx];
		check( Connection );
		check( Connection->State == USOCK_Pending || Connection->State == USOCK_Open || Connection->State == USOCK_Closed );
		checkSlow( Connection->GetUChildConnection() == NULL );

		// Handle not ready channels.
		//@note: we cannot add Connection->IsNetReady(0) here to check for saturation, as if that's the case we still want to figure out the list of relevant actors
		//			to reset their NetUpdateTime so that they will get sent as soon as the connection is no longer saturated
		AActor* OwningActor = Connection->OwningActor;
		if ( OwningActor != NULL && Connection->State == USOCK_Open && ( Connection->Driver->GetElapsedTime() - Connection->LastReceiveTime < 1.5 ) )
		{
			check( World == OwningActor->GetWorld() );

			bFoundReadyConnection = true;

			// the view target is what the player controller is looking at OR the owning actor itself when using beacons
			AActor* DesiredViewTarget = OwningActor;
			if (Connection->PlayerController)
			{
				if (AActor* ViewTarget = Connection->PlayerController->GetViewTarget())
				{
					if (ViewTarget->GetWorld())
					{
						// It is safe to use the player controller's view target.
						DesiredViewTarget = ViewTarget;
					}
					else
					{
						// Log an error, since this means the view target for the player controller no longer has a valid world (this can happen
						// if the player controller's view target was in a sublevel instance that has been unloaded).
						UE_LOG(LogNet, Warning, TEXT("Player controller %s's view target (%s) no longer has a valid world! Was it unloaded as part a level instance?"),
							*GetNameSafe(Connection->PlayerController), *GetNameSafe(ViewTarget));
					}
				}
			}
			Connection->ViewTarget = DesiredViewTarget;

			for ( int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++ )
			{
				UNetConnection *Child = Connection->Children[ChildIdx];
				APlayerController* ChildPlayerController = Child->PlayerController;
				if ( ChildPlayerController != NULL )
				{
					Child->ViewTarget = ChildPlayerController->GetViewTarget();
				}
				else
				{
					Child->ViewTarget = NULL;
				}
			}
		}
		else
		{
			Connection->ViewTarget = NULL;
			for ( int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++ )
			{
				Connection->Children[ChildIdx]->ViewTarget = NULL;
			}
		}
	}

	return bFoundReadyConnection ? NumClientsToTick : 0;
}

void UNetDriver::ServerReplicateActors_BuildConsiderList( TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime )
{
	SCOPE_CYCLE_COUNTER( STAT_NetConsiderActorsTime );

	UE_LOG( LogNetTraffic, Log, TEXT( "ServerReplicateActors_BuildConsiderList, Building ConsiderList %4.2f" ), World->GetTimeSeconds() );

	int32 NumInitiallyDormant = 0;

	const bool bUseAdapativeNetFrequency = IsAdaptiveNetUpdateFrequencyEnabled();

	TArray<AActor*> ActorsToRemove;

	for ( const TSharedPtr<FNetworkObjectInfo>& ObjectInfo : GetNetworkObjectList().GetActiveObjects() )
	{
		FNetworkObjectInfo* ActorInfo = ObjectInfo.Get();

		if ( !ActorInfo->bPendingNetUpdate && World->TimeSeconds <= ActorInfo->NextUpdateTime )
		{
			continue;		// It's not time for this actor to perform an update, skip it
		}

		AActor* Actor = ActorInfo->Actor;

		if ( Actor->IsPendingKillPending() )
		{
			// Actors aren't allowed to be placed in the NetworkObjectList if they are PendingKillPending.
			// Actors should also be unconditionally removed from the NetworkObjectList when UWorld::DestroyActor is called.
			// If this is happening, it means code is not destructing Actors properly, and that's not OK.
			UE_LOG( LogNet, Warning, TEXT( "Actor %s was found in the NetworkObjectList, but is PendingKillPending" ), *GetNameSafe(Actor) );
			ActorsToRemove.Add( Actor );
			continue;
		}

		if ( Actor->GetRemoteRole() == ROLE_None )
		{
			ActorsToRemove.Add( Actor );
			continue;
		}

		// This actor may belong to a different net driver, make sure this is the correct one
		// (this can happen when using beacon net drivers for example)
		if (Actor->GetNetDriverName() != NetDriverName)
		{
			FString ActorNetDriverNameString;
			Actor->GetNetDriverName().ToString(ActorNetDriverNameString);

			FString NetDriverNameString;
			NetDriverName.ToString(NetDriverNameString);

			UE_LOG(LogNetTraffic, Error, TEXT("Actor %s in wrong network actors list! (Has net driver '%s', expected '%s')"),
					*GetNameSafe(Actor), *ActorNetDriverNameString, *NetDriverNameString);

			continue;
		}

		// Verify the actor is actually initialized (it might have been intentionally spawn deferred until a later frame)
		if ( !Actor->IsActorInitialized() )
		{
			continue;
		}

		// Don't send actors that may still be streaming in or out
		ULevel* Level = Actor->GetLevel();
		if ( Level->HasVisibilityChangeRequestPending() || Level->bIsAssociatingLevel )
		{
			continue;
		}

		if ( IsDormInitialStartupActor(Actor) )
		{
			// This stat isn't that useful in its current form when using NetworkActors list
			// We'll want to track initially dormant actors some other way to track them with stats
			SCOPE_CYCLE_COUNTER( STAT_NetInitialDormantCheckTime );
			NumInitiallyDormant++;
			ActorsToRemove.Add( Actor );
			//UE_LOG(LogNetTraffic, Log, TEXT("Skipping Actor %s - its initially dormant!"), *Actor->GetName() );
			continue;
		}

		checkSlow( Actor->NeedsLoadForClient() ); // We have no business sending this unless the client can load
		checkSlow( World == Actor->GetWorld() );

		// Set defaults if this actor is replicating for first time
		if ( ActorInfo->LastNetReplicateTime == 0 )
		{
			ActorInfo->LastNetReplicateTime = World->TimeSeconds;
			ActorInfo->OptimalNetUpdateDelta = 1.0f / Actor->NetUpdateFrequency;
		}

		const float ScaleDownStartTime = 2.0f;
		const float ScaleDownTimeRange = 5.0f;

		const float LastReplicateDelta = World->TimeSeconds - ActorInfo->LastNetReplicateTime;

		if ( LastReplicateDelta > ScaleDownStartTime )
		{
			if ( Actor->MinNetUpdateFrequency == 0.0f )
			{
				Actor->MinNetUpdateFrequency = 2.0f;
			}

			// Calculate min delta (max rate actor will update), and max delta (slowest rate actor will update)
			const float MinOptimalDelta = 1.0f / Actor->NetUpdateFrequency;									  // Don't go faster than NetUpdateFrequency
			const float MaxOptimalDelta = FMath::Max( 1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta ); // Don't go slower than MinNetUpdateFrequency (or NetUpdateFrequency if it's slower)

			// Interpolate between MinOptimalDelta/MaxOptimalDelta based on how long it's been since this actor actually sent anything
			const float Alpha = FMath::Clamp( ( LastReplicateDelta - ScaleDownStartTime ) / ScaleDownTimeRange, 0.0f, 1.0f );
			ActorInfo->OptimalNetUpdateDelta = FMath::Lerp( MinOptimalDelta, MaxOptimalDelta, Alpha );
		}

		// Setup ActorInfo->NextUpdateTime, which will be the next time this actor will replicate properties to connections
		// NOTE - We don't do this if bPendingNetUpdate is true, since this means we're forcing an update due to at least one connection
		//	that wasn't to replicate previously (due to saturation, etc)
		// NOTE - This also means all other connections will force an update (even if they just updated, we should look into this)
		if ( !ActorInfo->bPendingNetUpdate )
		{
			UE_LOG( LogNetTraffic, Log, TEXT( "actor %s requesting new net update, time: %2.3f" ), *GetNameSafe(Actor), World->TimeSeconds );

			const float NextUpdateDelta = bUseAdapativeNetFrequency ? ActorInfo->OptimalNetUpdateDelta : 1.0f / Actor->NetUpdateFrequency;

			// then set the next update time
			ActorInfo->NextUpdateTime = World->TimeSeconds + UpdateDelayRandomStream.FRand() * ServerTickTime + NextUpdateDelta;

			// and mark when the actor first requested an update
			//@note: using Time because it's compared against UActorChannel.LastUpdateTime which also uses that value
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			ActorInfo->LastNetUpdateTime = ElapsedTime;
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
			ActorInfo->LastNetUpdateTimestamp = ElapsedTime;
		}

		// and clear the pending update flag assuming all clients will be able to consider it
		ActorInfo->bPendingNetUpdate = false;

		// add it to the list to consider below
		// For performance reasons, make sure we don't resize the array. It should already be appropriately sized above!
		ensure( OutConsiderList.Num() < OutConsiderList.Max() );
		OutConsiderList.Add( ActorInfo );

		// Call PreReplication on all actors that will be considered
		Actor->CallPreReplication( this );
	}

	for ( AActor* Actor : ActorsToRemove )
	{
		RemoveNetworkActor( Actor );
	}

	// Update stats
	SET_DWORD_STAT( STAT_NumInitiallyDormantActors, NumInitiallyDormant );
	SET_DWORD_STAT( STAT_NumConsideredActors, OutConsiderList.Num() );
}

// Returns true if this actor should replicate to *any* of the passed in connections
static FORCEINLINE_DEBUGGABLE bool IsActorRelevantToConnection( const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers )
{
	for ( int32 viewerIdx = 0; viewerIdx < ConnectionViewers.Num(); viewerIdx++ )
	{
		if ( Actor->IsNetRelevantFor( ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, ConnectionViewers[viewerIdx].ViewLocation ) )
		{
			return true;
		}
	}

	return false;
}

// Returns true if this actor is owned by, and should replicate to *any* of the passed in connections
static FORCEINLINE_DEBUGGABLE UNetConnection* IsActorOwnedByAndRelevantToConnection( const AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, bool& bOutHasNullViewTarget )
{
	const AActor* ActorOwner = Actor->GetNetOwner();

	bOutHasNullViewTarget = false;

	for ( int i = 0; i < ConnectionViewers.Num(); i++ )
	{
		UNetConnection* ViewerConnection = ConnectionViewers[i].Connection;

		if ( ViewerConnection->ViewTarget == nullptr )
		{
			bOutHasNullViewTarget = true;
		}

		if ( ActorOwner == ViewerConnection->PlayerController ||
			 ( ViewerConnection->PlayerController && ActorOwner == ViewerConnection->PlayerController->GetPawn() ) ||
			 (ViewerConnection->ViewTarget && ViewerConnection->ViewTarget->IsRelevancyOwnerFor( Actor, ActorOwner, ViewerConnection->OwningActor ) ) )
		{
			return ViewerConnection;
		}
	}

	return nullptr;
}

// Returns true if this actor is considered dormant (and all properties caught up) to the current connection
static FORCEINLINE_DEBUGGABLE bool IsActorDormant( FNetworkObjectInfo* ActorInfo, const TWeakObjectPtr<UNetConnection>& Connection )
{
	// If actor is already dormant on this channel, then skip replication entirely
	return ActorInfo->DormantConnections.Contains( Connection );
}

// Returns true if this actor wants to go dormant for a particular connection
static FORCEINLINE_DEBUGGABLE bool ShouldActorGoDormant( AActor* Actor, const TArray<FNetViewer>& ConnectionViewers, UActorChannel* Channel, const float Time, const bool bLowNetBandwidth )
{
	if ( Actor->NetDormancy <= DORM_Awake || !Channel || Channel->bPendingDormancy || Channel->Dormant )
	{
		// Either shouldn't go dormant, or is already dormant
		return false;
	}

	if ( Actor->NetDormancy == DORM_DormantPartial )
	{
		for ( int32 viewerIdx = 0; viewerIdx < ConnectionViewers.Num(); viewerIdx++ )
		{
			if ( !Actor->GetNetDormancy( ConnectionViewers[viewerIdx].ViewLocation, ConnectionViewers[viewerIdx].ViewDir, ConnectionViewers[viewerIdx].InViewer, ConnectionViewers[viewerIdx].ViewTarget, Channel, Time, bLowNetBandwidth ) )
			{
				return false;
			}
		}
	}

	return true;
}

int32 UNetDriver::ServerReplicateActors_PrioritizeActors( UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors )
{
	SCOPE_CYCLE_COUNTER( STAT_NetPrioritizeActorsTime );

	// Get list of visible/relevant actors.

	NetTag++;

	// Set up to skip all sent temporary actors
	for ( int32 j = 0; j < Connection->SentTemporaries.Num(); j++ )
	{
		Connection->SentTemporaries[j]->NetTag = NetTag;
	}

	// Make list of all actors to consider.
	check( World == Connection->OwningActor->GetWorld() );

	int32 FinalSortedCount = 0;
	int32 DeletedCount = 0;

	// Make weak ptr once for IsActorDormant call
	TWeakObjectPtr<UNetConnection> WeakConnection(Connection);

	const int32 MaxSortedActors = ConsiderList.Num() + DestroyedStartupOrDormantActors.Num();
	if ( MaxSortedActors > 0 )
	{
		OutPriorityList = new ( FMemStack::Get(), MaxSortedActors ) FActorPriority;
		OutPriorityActors = new ( FMemStack::Get(), MaxSortedActors ) FActorPriority*;

		check( World == Connection->ViewTarget->GetWorld() );

		AGameNetworkManager* const NetworkManager = World->NetworkManager;
		const bool bLowNetBandwidth = NetworkManager ? NetworkManager->IsInLowBandwidthMode() : false;

		for ( FNetworkObjectInfo* ActorInfo : ConsiderList )
		{
			AActor* Actor = ActorInfo->Actor;

			UActorChannel* Channel = Connection->FindActorChannelRef( ActorInfo->WeakActor );

			// Skip actor if not relevant and theres no channel already.
			// Historically Relevancy checks were deferred until after prioritization because they were expensive (line traces).
			// Relevancy is now cheap and we are dealing with larger lists of considered actors, so we want to keep the list of
			// prioritized actors low.
			if (!Channel)
			{
				if (!IsLevelInitializedForActor(Actor, Connection))
				{
					// If the level this actor belongs to isn't loaded on client, don't bother sending
					continue;
				}

				if (!IsActorRelevantToConnection(Actor, ConnectionViewers))
				{
					// If not relevant (and we don't have a channel), skip
					continue;
				}
			}

			UNetConnection* PriorityConnection = Connection;

			if ( Actor->bOnlyRelevantToOwner )
			{
				// This actor should be owned by a particular connection, see if that connection is the one passed in
				bool bHasNullViewTarget = false;

				PriorityConnection = IsActorOwnedByAndRelevantToConnection( Actor, ConnectionViewers, bHasNullViewTarget );

				if ( PriorityConnection == nullptr )
				{
					// Not owned by this connection, if we have a channel, close it, and continue
					// NOTE - We won't close the channel if any connection has a NULL view target.
					//	This is to give all connections a chance to own it
					if ( !bHasNullViewTarget && Channel != NULL && ElapsedTime - Channel->RelevantTime >= RelevantTimeout )
					{
						Channel->Close(EChannelCloseReason::Relevancy);
					}

					// This connection doesn't own this actor
					continue;
				}
			}
			else if ( GSetNetDormancyEnabled != 0 )
			{
				// Skip Actor if dormant
				if ( IsActorDormant( ActorInfo, WeakConnection ) )
				{
					continue;
				}

				// See of actor wants to try and go dormant
				if ( ShouldActorGoDormant( Actor, ConnectionViewers, Channel, ElapsedTime, bLowNetBandwidth ) )
				{
					// Channel is marked to go dormant now once all properties have been replicated (but is not dormant yet)
					Channel->StartBecomingDormant();
				}
			}

			// Actor is relevant to this connection, add it to the list
			// NOTE - We use NetTag to make sure SentTemporaries didn't already mark this actor to be skipped
			if ( Actor->NetTag != NetTag )
			{
				UE_LOG( LogNetTraffic, Log, TEXT( "Consider %s alwaysrelevant %d frequency %f " ), *GetNameSafe(Actor), Actor->bAlwaysRelevant, Actor->NetUpdateFrequency );

				Actor->NetTag = NetTag;

				OutPriorityList[FinalSortedCount] = FActorPriority( PriorityConnection, Channel, ActorInfo, ConnectionViewers, bLowNetBandwidth );
				OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;

				FinalSortedCount++;

				if ( DebugRelevantActors )
				{
					LastPrioritizedActors.Add( Actor );
				}
			}
		}

		// Add in deleted actors
		for ( auto It = Connection->GetDestroyedStartupOrDormantActorGUIDs().CreateConstIterator(); It; ++It )
		{
			FActorDestructionInfo& DInfo = *DestroyedStartupOrDormantActors.FindChecked( *It );
			OutPriorityList[FinalSortedCount] = FActorPriority( Connection, &DInfo, ConnectionViewers );
			OutPriorityActors[FinalSortedCount] = OutPriorityList + FinalSortedCount;
			FinalSortedCount++;
			DeletedCount++;
		}

		// Sort by priority
		Sort( OutPriorityActors, FinalSortedCount, FCompareFActorPriority() );
	}

	UE_LOG( LogNetTraffic, Log, TEXT( "ServerReplicateActors_PrioritizeActors: Potential %04i ConsiderList %03i FinalSortedCount %03i" ), MaxSortedActors, ConsiderList.Num(), FinalSortedCount );

	// Setup stats
	SET_DWORD_STAT( STAT_PrioritizedActors, FinalSortedCount );
	SET_DWORD_STAT( STAT_NumRelevantDeletedActors, DeletedCount );

	return FinalSortedCount;
}

int32 UNetDriver::ServerReplicateActors_ProcessPrioritizedActors( UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated )
{
	SCOPE_CYCLE_COUNTER(STAT_NetProcessPrioritizedActorsTime);

	int32 ActorUpdatesThisConnection		= 0;
	int32 ActorUpdatesThisConnectionSent	= 0;
	int32 FinalRelevantCount				= 0;

	if ( !Connection->IsNetReady( 0 ) )
	{
		GNumSaturatedConnections++;
		// Connection saturated, don't process any actors
		return 0;
	}

	for ( int32 j = 0; j < FinalSortedCount; j++ )
	{
		FNetworkObjectInfo*	ActorInfo = PriorityActors[j]->ActorInfo;

		// Deletion entry
		if ( ActorInfo == NULL && PriorityActors[j]->DestructionInfo )
		{
			// Make sure client has streaming level loaded
			if ( PriorityActors[j]->DestructionInfo->StreamingLevelName != NAME_None && !Connection->ClientVisibleLevelNames.Contains( PriorityActors[j]->DestructionInfo->StreamingLevelName ) )
			{
				// This deletion entry is for an actor in a streaming level the connection doesn't have loaded, so skip it
				continue;
			}

			FinalRelevantCount++;
			UE_LOG( LogNetTraffic, Log, TEXT( "Server replicate actor creating destroy channel for NetGUID <%s,%s> Priority: %d" ), *PriorityActors[j]->DestructionInfo->NetGUID.ToString(), *PriorityActors[j]->DestructionInfo->PathName, PriorityActors[j]->Priority );

			SendDestructionInfo(Connection, PriorityActors[j]->DestructionInfo);

			Connection->RemoveDestructionInfo( PriorityActors[j]->DestructionInfo );		// Remove from connections to-be-destroyed list (close bunch of reliable, so it will make it there)
			continue;
		}

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
		static IConsoleVariable* DebugObjectCvar = IConsoleManager::Get().FindConsoleVariable( TEXT( "net.PackageMap.DebugObject" ) );
		static IConsoleVariable* DebugAllObjectsCvar = IConsoleManager::Get().FindConsoleVariable( TEXT( "net.PackageMap.DebugAll" ) );
		if ( ActorInfo &&
			 ( ( DebugObjectCvar && !DebugObjectCvar->GetString().IsEmpty() && GetNameSafe(ActorInfo->Actor).Contains( DebugObjectCvar->GetString() ) ) ||
			   ( DebugAllObjectsCvar && DebugAllObjectsCvar->GetInt() != 0 ) ) )
		{
			UE_LOG( LogNetPackageMap, Log, TEXT( "Evaluating actor for replication %s" ), *GetNameSafe(ActorInfo->Actor) );
		}
#endif

		// Normal actor replication
		UActorChannel* Channel = PriorityActors[j]->Channel;
		UE_LOG( LogNetTraffic, Log, TEXT( " Maybe Replicate %s" ), *GetNameSafe(ActorInfo->Actor) );
		if ( !Channel || Channel->Actor ) //make sure didn't just close this channel
		{
			AActor* Actor = ActorInfo->Actor;
			bool bIsRelevant = false;

			const bool bLevelInitializedForActor = IsLevelInitializedForActor( Actor, Connection );

			// only check visibility on already visible actors every 1.0 + 0.5R seconds
			// bTearOff actors should never be checked
			if ( bLevelInitializedForActor )
			{
				if ( !Actor->GetTearOff() && ( !Channel || ElapsedTime - Channel->RelevantTime > 1.0 ) )
				{
					if ( IsActorRelevantToConnection( Actor, ConnectionViewers ) )
					{
						bIsRelevant = true;
					}
					else if ( DebugRelevantActors )
					{
						LastNonRelevantActors.Add( Actor );
					}
				}
			}
			else
			{
				// Actor is no longer relevant because the world it is/was in is not loaded by client
				// exception: player controllers should never show up here
				UE_LOG( LogNetTraffic, Log, TEXT( "- Level not initialized for actor %s" ), *GetNameSafe(Actor) );
			}

			// if the actor is now relevant or was recently relevant
			const bool bIsRecentlyRelevant = bIsRelevant || ( Channel && ElapsedTime - Channel->RelevantTime < RelevantTimeout ) || (ActorInfo->ForceRelevantFrame >= Connection->LastProcessedFrame);

			if ( bIsRecentlyRelevant )
			{
				FinalRelevantCount++;

				TOptional<FScopedActorRoleSwap> SwapGuard;
				if (ActorInfo->bSwapRolesOnReplicate)
				{
					SwapGuard = FScopedActorRoleSwap(Actor);
				}

				// Find or create the channel for this actor.
				// we can't create the channel if the client is in a different world than we are
				// or the package map doesn't support the actor's class/archetype (or the actor itself in the case of serializable actors)
				// or it's an editor placed actor and the client hasn't initialized the level it's in
				if ( Channel == NULL && GuidCache->SupportsObject( Actor->GetClass() ) && GuidCache->SupportsObject( Actor->IsNetStartupActor() ? Actor : Actor->GetArchetype() ) )
				{
					if ( bLevelInitializedForActor )
					{
						// Create a new channel for this actor.
						Channel = (UActorChannel*)Connection->CreateChannelByName( NAME_Actor, EChannelCreateFlags::OpenedLocally );
						if ( Channel )
						{
							Channel->SetChannelActor(Actor, ESetChannelActorFlags::None);
						}
					}
					// if we couldn't replicate it for a reason that should be temporary, and this Actor is updated very infrequently, make sure we update it again soon
					else if ( Actor->NetUpdateFrequency < 1.0f )
					{
						UE_LOG( LogNetTraffic, Log, TEXT( "Unable to replicate %s" ), *GetNameSafe(Actor) );
						ActorInfo->NextUpdateTime = World->TimeSeconds + 0.2f * FMath::FRand();
					}
				}

				if ( Channel )
				{
					// if it is relevant then mark the channel as relevant for a short amount of time
					if ( bIsRelevant )
					{
						Channel->RelevantTime = ElapsedTime + 0.5 * UpdateDelayRandomStream.FRand();
					}
					// if the channel isn't saturated
					if ( Channel->IsNetReady( 0 ) )
					{
						// replicate the actor
						UE_LOG( LogNetTraffic, Log, TEXT( "- Replicate %s. %d" ), *GetNameSafe(Actor), PriorityActors[j]->Priority );
						if ( DebugRelevantActors )
						{
							LastRelevantActors.Add( Actor );
						}

						double ChannelLastNetUpdateTime = Channel->LastUpdateTime;

						if ( Channel->ReplicateActor() )
						{
#if USE_SERVER_PERF_COUNTERS
							// A channel time of 0.0 means this is the first time the actor is being replicated, so we don't need to record it
							if (ChannelLastNetUpdateTime > 0.0)
							{
								Connection->GetActorsStarvedByClassTimeMap().FindOrAdd(Actor->GetClass()->GetName()).Add((World->RealTimeSeconds - ChannelLastNetUpdateTime) * 1000.0f);
							}
#endif

							ActorUpdatesThisConnectionSent++;
							if ( DebugRelevantActors )
							{
								LastSentActors.Add( Actor );
							}

							// Calculate min delta (max rate actor will upate), and max delta (slowest rate actor will update)
							const float MinOptimalDelta				= 1.0f / Actor->NetUpdateFrequency;
							const float MaxOptimalDelta				= FMath::Max( 1.0f / Actor->MinNetUpdateFrequency, MinOptimalDelta );
							const float DeltaBetweenReplications	= ( World->TimeSeconds - ActorInfo->LastNetReplicateTime );

							// Choose an optimal time, we choose 70% of the actual rate to allow frequency to go up if needed
							ActorInfo->OptimalNetUpdateDelta = FMath::Clamp( DeltaBetweenReplications * 0.7f, MinOptimalDelta, MaxOptimalDelta );
							ActorInfo->LastNetReplicateTime = World->TimeSeconds;
						}
						ActorUpdatesThisConnection++;
						OutUpdated++;
					}
					else
					{
						UE_LOG( LogNetTraffic, Log, TEXT( "- Channel saturated, forcing pending update for %s" ), *GetNameSafe(Actor) );
						// otherwise force this actor to be considered in the next tick again
						Actor->ForceNetUpdate();
					}
					// second check for channel saturation
					if ( !Connection->IsNetReady( 0 ) )
					{
						// We can bail out now since this connection is saturated, we'll return how far we got though
						GNumSaturatedConnections++;
						return j;
					}
				}
			}

			// If the actor wasn't recently relevant, or if it was torn off, close the actor channel if it exists for this connection
			if ( ( !bIsRecentlyRelevant || Actor->GetTearOff() ) && Channel != NULL )
			{
				// Non startup (map) actors have their channels closed immediately, which destroys them.
				// Startup actors get to keep their channels open.

				// Fixme: this should be a setting
				if ( !bLevelInitializedForActor || !Actor->IsNetStartupActor() )
				{
					UE_LOG( LogNetTraffic, Log, TEXT( "- Closing channel for no longer relevant actor %s" ), *GetNameSafe(Actor) );
					Channel->Close(Actor->GetTearOff() ? EChannelCloseReason::TearOff : EChannelCloseReason::Relevancy);
				}
			}
		}
	}

	return FinalSortedCount;
}

// -------------------------------------------------------------------------------------------------------------------------
//	ServerReplicateActors: this is main function to replicate actors to client connections. It can be "outsourced" to a Replication Driver.
// -------------------------------------------------------------------------------------------------------------------------

int32 UNetDriver::ServerReplicateActors(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_NetServerRepActorsTime);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(ServerReplicateActors);

	if ( ClientConnections.Num() == 0 )
	{
		return 0;
	}

	SET_DWORD_STAT(STAT_NumReplicatedActors, 0);
	SET_DWORD_STAT(STAT_NumReplicatedActorBytes, 0);

#if CSV_PROFILER
	FScopedNetDriverStats NetDriverStats(OutBytes, this);
	GNumClientConnections = ClientConnections.Num();
#endif

	if (ReplicationDriver)
	{
		return ReplicationDriver->ServerReplicateActors(DeltaSeconds);
	}

	check( World );

	// Bump the ReplicationFrame value to invalidate any properties marked as "unchanged" for this frame.
	ReplicationFrame++;

	int32 Updated = 0;

	const int32 NumClientsToTick = ServerReplicateActors_PrepConnections( DeltaSeconds );

	if ( NumClientsToTick == 0 )
	{
		// No connections are ready this frame
		return 0;
	}

	AWorldSettings* WorldSettings = World->GetWorldSettings();

	bool bCPUSaturated		= false;
	float ServerTickTime	= GEngine->GetMaxTickRate( DeltaSeconds );
	if ( ServerTickTime == 0.f )
	{
		ServerTickTime = DeltaSeconds;
	}
	else
	{
		ServerTickTime	= 1.f/ServerTickTime;
		bCPUSaturated	= DeltaSeconds > 1.2f * ServerTickTime;
	}

	TArray<FNetworkObjectInfo*> ConsiderList;
	ConsiderList.Reserve( GetNetworkObjectList().GetActiveObjects().Num() );

	// Build the consider list (actors that are ready to replicate)
	ServerReplicateActors_BuildConsiderList( ConsiderList, ServerTickTime );

	TSet<UNetConnection*> ConnectionsToClose;

	FMemMark Mark( FMemStack::Get() );

	for ( int32 i=0; i < ClientConnections.Num(); i++ )
	{
		UNetConnection* Connection = ClientConnections[i];
		check(Connection);

		// net.DormancyValidate can be set to 2 to validate all dormant actors against last known state before going dormant
		if ( GNetDormancyValidate == 2 )
		{
			// TODO: DormantReplicatorMap will actually contain all Actors and Subobjects.
			// This means that we will call FObjectReplicator::ValidateAgainstState multiple times for
			// the same object (once for itself and again for each subobject).
			for ( auto It = Connection->DormantReplicatorMap.CreateIterator(); It; ++It )
			{
				FObjectReplicator& Replicator = It.Value().Get();

				if ( Replicator.OwningChannel != nullptr )
				{
					Replicator.ValidateAgainstState( Replicator.OwningChannel->GetActor() );
				}
			}
		}

		// if this client shouldn't be ticked this frame
		if (i >= NumClientsToTick)
		{
			//UE_LOG(LogNet, Log, TEXT("skipping update to %s"),*Connection->GetName());
			// then mark each considered actor as bPendingNetUpdate so that they will be considered again the next frame when the connection is actually ticked
			for (int32 ConsiderIdx = 0; ConsiderIdx < ConsiderList.Num(); ConsiderIdx++)
			{
				AActor *Actor = ConsiderList[ConsiderIdx]->Actor;
				// if the actor hasn't already been flagged by another connection,
				if (Actor != NULL && !ConsiderList[ConsiderIdx]->bPendingNetUpdate)
				{
					// find the channel
					UActorChannel *Channel = Connection->FindActorChannelRef(ConsiderList[ConsiderIdx]->WeakActor);
					// and if the channel last update time doesn't match the last net update time for the actor
					if (Channel != NULL && Channel->LastUpdateTime < ConsiderList[ConsiderIdx]->LastNetUpdateTimestamp)
					{
						//UE_LOG(LogNet, Log, TEXT("flagging %s for a future update"),*Actor->GetName());
						// flag it for a pending update
						ConsiderList[ConsiderIdx]->bPendingNetUpdate = true;
					}
				}
			}
			// clear the time sensitive flag to avoid sending an extra packet to this connection
			Connection->TimeSensitive = false;
		}
		else if (Connection->ViewTarget)
		{

			const int32 LocalNumSaturated = GNumSaturatedConnections;

			// Make a list of viewers this connection should consider (this connection and children of this connection)
			TArray<FNetViewer>& ConnectionViewers = WorldSettings->ReplicationViewers;

			ConnectionViewers.Reset();
			new( ConnectionViewers )FNetViewer( Connection, DeltaSeconds );
			for ( int32 ViewerIndex = 0; ViewerIndex < Connection->Children.Num(); ViewerIndex++ )
			{
				if ( Connection->Children[ViewerIndex]->ViewTarget != NULL )
				{
					new( ConnectionViewers )FNetViewer( Connection->Children[ViewerIndex], DeltaSeconds );
				}
			}

			// send ClientAdjustment if necessary
			// we do this here so that we send a maximum of one per packet to that client; there is no value in stacking additional corrections
			if ( Connection->PlayerController )
			{
				Connection->PlayerController->SendClientAdjustment();
			}

			for ( int32 ChildIdx = 0; ChildIdx < Connection->Children.Num(); ChildIdx++ )
			{
				if ( Connection->Children[ChildIdx]->PlayerController != NULL )
				{
					Connection->Children[ChildIdx]->PlayerController->SendClientAdjustment();
				}
			}

			FMemMark RelevantActorMark(FMemStack::Get());

			FActorPriority* PriorityList	= NULL;
			FActorPriority** PriorityActors = NULL;

			// Get a sorted list of actors for this connection
			const int32 FinalSortedCount = ServerReplicateActors_PrioritizeActors( Connection, ConnectionViewers, ConsiderList, bCPUSaturated, PriorityList, PriorityActors );

			// Process the sorted list of actors for this connection
			const int32 LastProcessedActor = ServerReplicateActors_ProcessPrioritizedActors( Connection, ConnectionViewers, PriorityActors, FinalSortedCount, Updated );

			// relevant actors that could not be processed this frame are marked to be considered for next frame
			for ( int32 k=LastProcessedActor; k<FinalSortedCount; k++ )
			{
				if (!PriorityActors[k]->ActorInfo)
				{
					// A deletion entry, skip it because we dont have anywhere to store a 'better give higher priority next time'
					continue;
				}

				AActor* Actor = PriorityActors[k]->ActorInfo->Actor;

				UActorChannel* Channel = PriorityActors[k]->Channel;

				UE_LOG(LogNetTraffic, Verbose, TEXT("Saturated. %s"), *GetNameSafe(Actor));
				if (Channel != NULL && ElapsedTime - Channel->RelevantTime <= 1.0)
				{
					UE_LOG(LogNetTraffic, Log, TEXT(" Saturated. Mark %s NetUpdateTime to be checked for next tick"), *GetNameSafe(Actor));
					PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
				}
				else if ( IsActorRelevantToConnection( Actor, ConnectionViewers ) )
				{
					// If this actor was relevant but didn't get processed, force another update for next frame
					UE_LOG( LogNetTraffic, Log, TEXT( " Saturated. Mark %s NetUpdateTime to be checked for next tick" ), *GetNameSafe(Actor) );
					PriorityActors[k]->ActorInfo->bPendingNetUpdate = true;
					if ( Channel != NULL )
					{
						Channel->RelevantTime = ElapsedTime + 0.5 * UpdateDelayRandomStream.FRand();
					}
					}

				// If the actor was forced to relevant and didn't get processed, try again on the next update;
				if (PriorityActors[k]->ActorInfo->ForceRelevantFrame >= Connection->LastProcessedFrame)
				{
					PriorityActors[k]->ActorInfo->ForceRelevantFrame = ReplicationFrame+1;
				}
			}
			RelevantActorMark.Pop();

			ConnectionViewers.Reset();

			Connection->LastProcessedFrame = ReplicationFrame;

			const bool bWasSaturated = GNumSaturatedConnections > LocalNumSaturated;
			Connection->TrackReplicationForAnalytics(bWasSaturated);
		}

		if (Connection->GetPendingCloseDueToReplicationFailure())
		{
			ConnectionsToClose.Add(Connection);
		}
	}

	// shuffle the list of connections if not all connections were ticked
	if (NumClientsToTick < ClientConnections.Num())
	{
		int32 NumConnectionsToMove = NumClientsToTick;
		while (NumConnectionsToMove > 0)
		{
			// move all the ticked connections to the end of the list so that the other connections are considered first for the next frame
			UNetConnection *Connection = ClientConnections[0];
			ClientConnections.RemoveAt(0,1);
			ClientConnections.Add(Connection);
			NumConnectionsToMove--;
		}
	}
	Mark.Pop();

	if (DebugRelevantActors)
	{
		PrintDebugRelevantActors();
		LastPrioritizedActors.Empty();
		LastSentActors.Empty();
		LastRelevantActors.Empty();
		LastNonRelevantActors.Empty();

		DebugRelevantActors  = false;
	}

	for (UNetConnection* ConnectionToClose : ConnectionsToClose)
	{
		ConnectionToClose->Close();
	}

	return Updated;
}

void UNetDriver::PrintDebugRelevantActors()
{
	struct SLocal
	{
		static void AggregateAndPrint( TArray< TWeakObjectPtr<AActor> >	&List, FString txt )
		{
			TMap< TWeakObjectPtr<UClass>, int32>	ClassSummary;
			TMap< TWeakObjectPtr<UClass>, int32>	SuperClassSummary;

			for (auto It = List.CreateIterator(); It; ++It)
			{
				if (AActor* Actor = It->Get())
				{

					ClassSummary.FindOrAdd(Actor->GetClass())++;
					if (Actor->GetClass()->GetSuperStruct())
					{
						SuperClassSummary.FindOrAdd( Actor->GetClass()->GetSuperClass() )++;
					}
				}
			}

			struct FCompareActorClassCount
			{
				FORCEINLINE bool operator()( int32 A, int32 B ) const
				{
					return A < B;
				}
			};


			ClassSummary.ValueSort(FCompareActorClassCount());
			SuperClassSummary.ValueSort(FCompareActorClassCount());

			UE_LOG(LogNet, Warning, TEXT("------------------------------") );
			UE_LOG(LogNet, Warning, TEXT(" %s Class Summary"), *txt );
			UE_LOG(LogNet, Warning, TEXT("------------------------------") );

			for (auto It = ClassSummary.CreateIterator(); It; ++It)
			{
				UE_LOG(LogNet, Warning, TEXT("%4d - %s (%s)"), It.Value(), *GetNameSafe(It.Key().Get()), It.Key()->GetSuperStruct() ? *GetNameSafe(It.Key()->GetSuperStruct()) : TEXT("NULL") );
			}

			UE_LOG(LogNet, Warning, TEXT("---------------------------------") );
			UE_LOG(LogNet, Warning, TEXT(" %s Parent Class Summary "), *txt );
			UE_LOG(LogNet, Warning, TEXT("------------------------------") );

			for (auto It = SuperClassSummary.CreateIterator(); It; ++It)
			{
				UE_LOG(LogNet, Warning, TEXT("%4d - %s (%s)"), It.Value(), *GetNameSafe(It.Key().Get()), It.Key()->GetSuperStruct() ? *GetNameSafe(It.Key()->GetSuperStruct()) : TEXT("NULL") );
			}

			UE_LOG(LogNet, Warning, TEXT("---------------------------------") );
			UE_LOG(LogNet, Warning, TEXT(" %s Total: %d"), *txt, List.Num() );
			UE_LOG(LogNet, Warning, TEXT("---------------------------------") );
		}
	};

	SLocal::AggregateAndPrint( LastPrioritizedActors, TEXT(" Prioritized Actor") );
	SLocal::AggregateAndPrint( LastRelevantActors, TEXT(" Relevant Actor") );
	SLocal::AggregateAndPrint( LastNonRelevantActors, TEXT(" NonRelevant Actor") );
	SLocal::AggregateAndPrint( LastSentActors, TEXT(" Sent Actor") );

	UE_LOG(LogNet, Warning, TEXT("---------------------------------") );
	UE_LOG(LogNet, Warning, TEXT(" Num Connections: %d"), ClientConnections.Num() );

	UE_LOG(LogNet, Warning, TEXT("---------------------------------") );

}

bool UNetDriver::IsDormInitialStartupActor(AActor* Actor)
{
	return Actor && Actor->IsNetStartupActor() && (Actor->NetDormancy == DORM_Initial);
}

void UNetDriver::TickFlushHook(UNetDriver* This, float DeltaSeconds)
{
	if (This->IsServer() && This->ClientConnections.Num() > 0 && !This->bSkipServerReplicateActors)
	{
		// Update all clients.
		int32 Updated = This->ServerReplicateActors( DeltaSeconds );

		static int32 LastUpdateCount = 0;
		// Only log the zero replicated actors once after replicating an actor
		if ((LastUpdateCount && !Updated) || Updated)
		{
			UE_LOG(LogNetTraffic, Verbose, TEXT("%s replicated %d actors"), *This->GetDescription(), Updated);
		}
		LastUpdateCount = Updated;
	}

	TickFlushOG(This, DeltaSeconds);
}

void UNetDriver::Init()
{
	Memory::HookDetour(ImageBase + 0xC41400, TickFlushHook, &TickFlushOG);
}

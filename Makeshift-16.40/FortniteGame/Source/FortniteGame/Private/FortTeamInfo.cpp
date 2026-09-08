#include "pch.h"

void AFortTeamInfo::SpotActorForTeam(AActor* ActorToSpot, AFortPlayerController* Spotter) {
	const IFortSpottableActorInterface* SpottableActor = ActorToSpot ? static_cast<const IFortSpottableActorInterface*>(ActorToSpot->GetInterfaceAddress(IFortSpottableActorInterface::StaticClass())) : nullptr;
	if (Role != ENetRole::ROLE_Authority || !SpottableActor || !Spotter) {
		return;
	}

	if (SpottableActor->CanBeSpottedBy(Spotter)) {
		TeamSpottedActors.AttemptSpotActor(ActorToSpot, Spotter);
	}
}

void FTeamSpottedActorInfoArray::AttemptSpotActor(AActor* ActorToSpot, AFortPlayerController* Spotter) {
	if (!ActorToSpot || !Spotter || !OwningTeam || OwningTeam->Role != ENetRole::ROLE_Authority) {
		return;
	}

	bool bAlreadySpotted = false;
	int32 SpotterInfoIdx = INDEX_NONE;
	for (int32 InfoIdx = 0; InfoIdx < SpottedActorInfo.Num(); ++InfoIdx) {
		const FTeamSpottedActorInfo& Info = SpottedActorInfo[InfoIdx];
		if (Info.SpottedActor == ActorToSpot) {
			bAlreadySpotted = true;
			if (SpotterInfoIdx != INDEX_NONE) {
				break;
			}
		}

		if (Info.Spotter == Spotter) {
			SpotterInfoIdx = InfoIdx;
			if (bAlreadySpotted) {
				break;
			}
		}
	}

	const FName OnSpottedActorDiedName(L"OnSpottedActorDied");
	if (!bAlreadySpotted) {
		if (SpotterInfoIdx != INDEX_NONE) {
			UnspotActorByIdx(SpotterInfoIdx);
		}

		FTeamSpottedActorInfo NewInfo{};
		NewInfo.Spotter = Spotter;
		NewInfo.SpottedActor = ActorToSpot;
		const int32 NewInfoIdx = SpottedActorInfo.Add(NewInfo);
		MarkItemDirty(SpottedActorInfo[NewInfoIdx]);
		SpottedActorInfo[NewInfoIdx].PostReplicatedAdd(*this);

		if (ActorToSpot->IsA(AFortPawn::StaticClass())) {
			FScriptDelegate OnSpottedActorDiedDelegate;
			OnSpottedActorDiedDelegate.Object = OwningTeam;
			OnSpottedActorDiedDelegate.FunctionName = OnSpottedActorDiedName;
			static_cast<AFortPawn*>(ActorToSpot)->OnDied.AddUnique(OnSpottedActorDiedDelegate);
		}
		return;
	}

	const int32 InfoIdx = FindSpottedActorInfo(ActorToSpot);
	if (InfoIdx == INDEX_NONE || SpottedActorInfo[InfoIdx].Spotter != Spotter) {
		return;
	}

	if (ActorToSpot->IsA(AFortPawn::StaticClass())) {
		static_cast<AFortPawn*>(ActorToSpot)->OnDied.Remove(OwningTeam, OnSpottedActorDiedName);
	}

	UnspotActorByIdx(InfoIdx);
}

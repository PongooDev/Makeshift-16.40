#include "pch.h"

static AController* GetOwningController(const UFortControllerComponent_IndicatedActorManagement* Component) {
	AActor* Owner = Component->GetOwner();
	return Owner ? Owner->Cast<AController>() : nullptr;
}

static UFortControllerComponent_IndicatedActorManagement* FindIndicatedActorManagementComponent(APlayerState* PlayerState) {
	AActor* Owner = PlayerState ? PlayerState->Owner : nullptr;
	AController* Controller = Owner ? Owner->Cast<AController>() : nullptr;
	return Controller ? Controller->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr;
}

static void GetComponentsToShareWith(AController* Controller, EShareActorWith ShareActorWith, TArray<UFortControllerComponent_IndicatedActorManagement*>& OutComponents) {
	AFortPlayerStateAthena* PlayerState = Controller && Controller->PlayerState ? Controller->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	AFortGameStateAthena* GameState = World && World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
	if (!PlayerState || !GameState) {
		return;
	}

	if (ShareActorWith == EShareActorWith::SquadOnTeam) {
		TArray<AFortPlayerStateAthena*> SquadMemberPlayerStates = GameState->GetSquadMemberPlayerStates(PlayerState, false);
		for (int32 Index = 0; Index < SquadMemberPlayerStates.Num(); ++Index) {
			if (UFortControllerComponent_IndicatedActorManagement* Component = FindIndicatedActorManagementComponent(SquadMemberPlayerStates[Index])) {
				OutComponents.Add(Component);
			}
		}
	}
	else if (ShareActorWith == EShareActorWith::AllTeam) {
		for (int32 Index = 0; Index < GameState->PlayerArray.Num(); ++Index) {
			AFortPlayerStateAthena* TeamMemberPlayerState = GameState->PlayerArray[Index] ? GameState->PlayerArray[Index]->Cast<AFortPlayerStateAthena>() : nullptr;
			if (!TeamMemberPlayerState || TeamMemberPlayerState == PlayerState || TeamMemberPlayerState->TeamIndex != PlayerState->TeamIndex) {
				continue;
			}

			if (UFortControllerComponent_IndicatedActorManagement* Component = FindIndicatedActorManagementComponent(TeamMemberPlayerState)) {
				OutComponents.Add(Component);
			}
		}
	}
}

template <typename EntryType, typename DataType>
static void AddActorToList(FFastArraySerializer& List, TArray<EntryType>& Entries, AActor* Actor, const DataType& Data, float StartTime, float EndTime, bool bAddAsUnique, bool bReplaceExistingEntry, bool bRefreshExistingEntry) {
	if (bAddAsUnique) {
		for (int32 Index = 0; Index < Entries.Num(); ++Index) {
			EntryType& ExistingEntry = Entries[Index];
			if (ExistingEntry.Actor != Actor || !(ExistingEntry.Data.GroupIdentifier == Data.GroupIdentifier)) {
				continue;
			}

			if (bReplaceExistingEntry) {
				ExistingEntry.StartTime = StartTime;
				ExistingEntry.EndTime = EndTime;
				ExistingEntry.Data = Data;
				ExistingEntry.bReplaceExistingWhenAdded = true;
				List.MarkItemDirty(ExistingEntry);
			}
			else if (bRefreshExistingEntry) {
				ExistingEntry.StartTime = StartTime;
				ExistingEntry.EndTime = EndTime;
				ExistingEntry.bRefreshExistingWhenAdded = true;
				List.MarkItemDirty(ExistingEntry);
			}
			return;
		}
	}

	EntryType NewEntry{};
	NewEntry.Actor = Actor;
	NewEntry.StartTime = StartTime;
	NewEntry.EndTime = EndTime;
	NewEntry.Data = Data;
	NewEntry.bReplaceExistingWhenAdded = bReplaceExistingEntry;
	NewEntry.bRefreshExistingWhenAdded = bRefreshExistingEntry;
	const int32 NewIndex = Entries.Add(NewEntry);
	List.MarkItemDirty(Entries[NewIndex]);
}

template <typename EntryType, typename PredicateType>
static void RemoveEntriesFromList(FFastArraySerializer& List, TArray<EntryType>& Entries, PredicateType Predicate) {
	bool bRemovedEntry = false;
	for (int32 Index = Entries.Num() - 1; Index >= 0; --Index) {
		if (Predicate(Entries[Index])) {
			Entries.RemoveAt(Index);
			bRemovedEntry = true;
		}
	}

	if (bRemovedEntry) {
		List.MarkArrayDirty();
	}
}

void UFortControllerComponent_IndicatedActorManagement::AddActorsToIndicatedList(TArray<AActor*>* IndicatedActors, const FIndicatedActorData& Data, const bool bAddAsUnique, const bool bAllowOwningPlayer, const bool bReplaceExistingEntry, const bool bRefreshExistingEntry) {
	AController* Controller = GetOwningController(this);
	UWorld* World = GetWorld();
	if (!IndicatedActors || !Controller || !World) {
		return;
	}

	const float StartTime = World->GetTimeSeconds();
	const float EndTime = Data.Duration > 0.f ? StartTime + Data.Duration : 0.f;

	TArray<AActor*> AddedActors;
	for (int32 Index = 0; Index < IndicatedActors->Num(); ++Index) {
		AActor* Actor = (*IndicatedActors)[Index];
		if (!Actor || (!bAllowOwningPlayer && (Actor == Controller || Actor == Controller->Pawn))) {
			continue;
		}

		AddActorToList(IndicatedActorList, IndicatedActorList.Entries, Actor, Data, StartTime, EndTime, bAddAsUnique, bReplaceExistingEntry, bRefreshExistingEntry);
		AddedActors.Add(Actor);
	}

	if (Data.ShareActorWith == EShareActorWith::None || AddedActors.Num() == 0) {
		return;
	}

	FIndicatedActorData SharedData = Data;
	SharedData.ShareActorWith = EShareActorWith::None;

	TArray<UFortControllerComponent_IndicatedActorManagement*> ComponentsToShareWith;
	GetComponentsToShareWith(Controller, Data.ShareActorWith, ComponentsToShareWith);
	for (int32 Index = 0; Index < ComponentsToShareWith.Num(); ++Index) {
		ComponentsToShareWith[Index]->AddActorsToIndicatedList(&AddedActors, SharedData, bAddAsUnique, bAllowOwningPlayer, bReplaceExistingEntry, bRefreshExistingEntry);
	}
}

void UFortControllerComponent_IndicatedActorManagement::AddActorsToStenciledList(TArray<AActor*>* StenciledActors, const FStenciledActorData& Data, const bool bAddAsUnique, const bool bReplaceExistingEntry, const bool bRefreshExistingEntry) {
	AController* Controller = GetOwningController(this);
	UWorld* World = GetWorld();
	if (!StenciledActors || !Controller || !World) {
		return;
	}

	const float StartTime = World->GetTimeSeconds();
	const float EndTime = Data.Duration > 0.f ? StartTime + Data.Duration : 0.f;

	TArray<AActor*> AddedActors;
	for (int32 Index = 0; Index < StenciledActors->Num(); ++Index) {
		AActor* Actor = (*StenciledActors)[Index];
		if (!Actor) {
			continue;
		}

		AddActorToList(StenciledActorList, StenciledActorList.Entries, Actor, Data, StartTime, EndTime, bAddAsUnique, bReplaceExistingEntry, bRefreshExistingEntry);
		AddedActors.Add(Actor);
	}

	if (Data.ShareActorWith == EShareActorWith::None || AddedActors.Num() == 0) {
		return;
	}

	FStenciledActorData SharedData = Data;
	SharedData.ShareActorWith = EShareActorWith::None;

	TArray<UFortControllerComponent_IndicatedActorManagement*> ComponentsToShareWith;
	GetComponentsToShareWith(Controller, Data.ShareActorWith, ComponentsToShareWith);
	for (int32 Index = 0; Index < ComponentsToShareWith.Num(); ++Index) {
		ComponentsToShareWith[Index]->AddActorsToStenciledList(&AddedActors, SharedData, bAddAsUnique, bReplaceExistingEntry, bRefreshExistingEntry);
	}
}

void UFortControllerComponent_IndicatedActorManagement::RemoveActorFromIndicatedList(AActor* IndicatedActor, const bool bIncludeSquad) {
	if (!IndicatedActor) {
		return;
	}

	RemoveEntriesFromList(IndicatedActorList, IndicatedActorList.Entries, [IndicatedActor](const FIndicatedActorInfoEntry& Entry) { return Entry.Actor == IndicatedActor; });

	if (bIncludeSquad) {
		TArray<UFortControllerComponent_IndicatedActorManagement*> SquadComponents;
		GetComponentsToShareWith(GetOwningController(this), EShareActorWith::SquadOnTeam, SquadComponents);
		for (int32 Index = 0; Index < SquadComponents.Num(); ++Index) {
			SquadComponents[Index]->RemoveActorFromIndicatedList(IndicatedActor, false);
		}
	}
}

void UFortControllerComponent_IndicatedActorManagement::RemoveActorFromStenciledList(AActor* StenciledActor, const bool bIncludeSquad) {
	if (!StenciledActor) {
		return;
	}

	RemoveEntriesFromList(StenciledActorList, StenciledActorList.Entries, [StenciledActor](const FStenciledActorInfoEntry& Entry) { return Entry.Actor == StenciledActor; });

	if (bIncludeSquad) {
		TArray<UFortControllerComponent_IndicatedActorManagement*> SquadComponents;
		GetComponentsToShareWith(GetOwningController(this), EShareActorWith::SquadOnTeam, SquadComponents);
		for (int32 Index = 0; Index < SquadComponents.Num(); ++Index) {
			SquadComponents[Index]->RemoveActorFromStenciledList(StenciledActor, false);
		}
	}
}

void UFortControllerComponent_IndicatedActorManagement::RemoveGroupFromIndicatedList(const FString& GroupIdentifier, const bool bIncludeSquad) {
	RemoveEntriesFromList(IndicatedActorList, IndicatedActorList.Entries, [&GroupIdentifier](const FIndicatedActorInfoEntry& Entry) { return Entry.Data.GroupIdentifier == GroupIdentifier; });

	if (bIncludeSquad) {
		TArray<UFortControllerComponent_IndicatedActorManagement*> SquadComponents;
		GetComponentsToShareWith(GetOwningController(this), EShareActorWith::SquadOnTeam, SquadComponents);
		for (int32 Index = 0; Index < SquadComponents.Num(); ++Index) {
			SquadComponents[Index]->RemoveGroupFromIndicatedList(GroupIdentifier, false);
		}
	}
}

void UFortControllerComponent_IndicatedActorManagement::RemoveGroupFromStenciledList(const FString& GroupIdentifier, const bool bIncludeSquad) {
	RemoveEntriesFromList(StenciledActorList, StenciledActorList.Entries, [&GroupIdentifier](const FStenciledActorInfoEntry& Entry) { return Entry.Data.GroupIdentifier == GroupIdentifier; });

	if (bIncludeSquad) {
		TArray<UFortControllerComponent_IndicatedActorManagement*> SquadComponents;
		GetComponentsToShareWith(GetOwningController(this), EShareActorWith::SquadOnTeam, SquadComponents);
		for (int32 Index = 0; Index < SquadComponents.Num(); ++Index) {
			SquadComponents[Index]->RemoveGroupFromStenciledList(GroupIdentifier, false);
		}
	}
}

DEFINE_FUNCTION(UFortControllerComponent_IndicatedActorManagement::execAddActorsToIndicatedList)
{
	P_GET_TARRAY_REF(AActor*,Z_Param_Out_IndicatedActors);
	P_GET_STRUCT(FIndicatedActorData,Z_Param_Data);
	P_GET_UBOOL(Z_Param_bAddAsUnique);
	P_GET_UBOOL(Z_Param_bAllowOwningPlayer);
	P_GET_UBOOL(Z_Param_bReplaceExistingEntry);
	P_GET_UBOOL(Z_Param_bRefreshExistingEntry);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(UFortControllerComponent_IndicatedActorManagement)->AddActorsToIndicatedList(&Z_Param_Out_IndicatedActors,Z_Param_Data,Z_Param_bAddAsUnique,Z_Param_bAllowOwningPlayer,Z_Param_bReplaceExistingEntry,Z_Param_bRefreshExistingEntry);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortControllerComponent_IndicatedActorManagement::execAddActorsToStenciledList)
{
	P_GET_TARRAY_REF(AActor*,Z_Param_Out_StenciledActors);
	P_GET_STRUCT(FStenciledActorData,Z_Param_Data);
	P_GET_UBOOL(Z_Param_bAddAsUnique);
	P_GET_UBOOL(Z_Param_bReplaceExistingEntry);
	P_GET_UBOOL(Z_Param_bRefreshExistingEntry);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(UFortControllerComponent_IndicatedActorManagement)->AddActorsToStenciledList(&Z_Param_Out_StenciledActors,Z_Param_Data,Z_Param_bAddAsUnique,Z_Param_bReplaceExistingEntry,Z_Param_bRefreshExistingEntry);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortControllerComponent_IndicatedActorManagement::execRemoveActorFromIndicatedList)
{
	P_GET_OBJECT(AActor,Z_Param_IndicatedActor);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(UFortControllerComponent_IndicatedActorManagement)->RemoveActorFromIndicatedList(Z_Param_IndicatedActor,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortControllerComponent_IndicatedActorManagement::execRemoveActorFromStenciledList)
{
	P_GET_OBJECT(AActor,Z_Param_StenciledActor);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(UFortControllerComponent_IndicatedActorManagement)->RemoveActorFromStenciledList(Z_Param_StenciledActor,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortControllerComponent_IndicatedActorManagement::execRemoveGroupFromIndicatedList)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_GroupIdentifier);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(UFortControllerComponent_IndicatedActorManagement)->RemoveGroupFromIndicatedList(Z_Param_GroupIdentifier,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortControllerComponent_IndicatedActorManagement::execRemoveGroupFromStenciledList)
{
	P_GET_PROPERTY(FStrProperty,Z_Param_GroupIdentifier);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(UFortControllerComponent_IndicatedActorManagement)->RemoveGroupFromStenciledList(Z_Param_GroupIdentifier,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

void UFortControllerComponent_IndicatedActorManagement::Init() {
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortControllerComponent_IndicatedActorManagement.AddActorsToIndicatedList"), execAddActorsToIndicatedList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortControllerComponent_IndicatedActorManagement.AddActorsToStenciledList"), execAddActorsToStenciledList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortControllerComponent_IndicatedActorManagement.RemoveActorFromIndicatedList"), execRemoveActorFromIndicatedList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortControllerComponent_IndicatedActorManagement.RemoveActorFromStenciledList"), execRemoveActorFromStenciledList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortControllerComponent_IndicatedActorManagement.RemoveGroupFromIndicatedList"), execRemoveGroupFromIndicatedList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortControllerComponent_IndicatedActorManagement.RemoveGroupFromStenciledList"), execRemoveGroupFromStenciledList);
}

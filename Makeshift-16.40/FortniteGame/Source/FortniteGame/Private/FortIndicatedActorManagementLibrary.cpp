#include "pch.h"

static UFortControllerComponent_IndicatedActorManagement* GetIndicatedActorManagementComponent(AController* InstigatingController) {
	return InstigatingController ? InstigatingController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr;
}

static void FindActorsInRadius(AController* InstigatingController, AActor* InstigatingActorOverride, const FIndicatedActorDataWithFilter& FilterData, TArray<AActor*>& OutActors) {
	AActor* InstigatingActor = InstigatingActorOverride ? InstigatingActorOverride : (InstigatingController ? static_cast<AActor*>(InstigatingController->Pawn) : nullptr);
	if (!InstigatingActor) {
		return;
	}

	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(InstigatingController, InstigatingActor->K2_GetActorLocation(), FilterData.OverlapRadius, FilterData.ObjectTypes, FilterData.ActorClassFilter, ActorsToIgnore, &OverlappedActors);

	for (int32 Index = 0; Index < OverlappedActors.Num(); ++Index) {
		AActor* Actor = OverlappedActors[Index];
		if (!Actor) {
			continue;
		}

		if (!FilterData.IndicatedActorTags.IsEmpty() || !FilterData.IndicatedActorTagQuery.IsEmpty()) {
			const IGameplayTagAssetInterface* TagInterface = static_cast<const IGameplayTagAssetInterface*>(Actor->GetInterfaceAddress(IGameplayTagAssetInterface::StaticClass()));
			if (!TagInterface) {
				continue;
			}

			FGameplayTagContainer OwnedTags;
			TagInterface->GetOwnedGameplayTags(OwnedTags);
			if (!FilterData.IndicatedActorTags.IsEmpty() && !OwnedTags.HasAll(FilterData.IndicatedActorTags)) {
				continue;
			}
			if (!FilterData.IndicatedActorTagQuery.IsEmpty() && !FilterData.IndicatedActorTagQuery.Matches(OwnedTags)) {
				continue;
			}
		}

		OutActors.Add(Actor);
	}
}

void UFortIndicatedActorManagementLibrary::AddActorsInRadiusToIndicatedList(AController* InstigatingController, const TArray<FIndicatedActorDataWithFilter>& IndicatedActorFilterDatas, const bool bAddAsUnique, const bool bReplaceExistingEntry, const bool bRefreshExistingEntry, AActor* InstigatingActorOverride) {
	UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController);
	if (!Component) {
		return;
	}

	for (int32 Index = 0; Index < IndicatedActorFilterDatas.Num(); ++Index) {
		TArray<AActor*> IndicatedActors;
		FindActorsInRadius(InstigatingController, InstigatingActorOverride, IndicatedActorFilterDatas[Index], IndicatedActors);
		if (IndicatedActors.Num() > 0) {
			Component->AddActorsToIndicatedList(&IndicatedActors, IndicatedActorFilterDatas[Index].IndicatedData, bAddAsUnique, false, bReplaceExistingEntry, bRefreshExistingEntry);
		}
	}
}

void UFortIndicatedActorManagementLibrary::AddActorsInRadiusToStenciledList(AController* InstigatingController, const TArray<FIndicatedActorDataWithFilter>& StenciledActorFilterDatas, const bool bAddAsUnique, const bool bReplaceExistingEntry, const bool bRefreshExistingEntry, AActor* InstigatingActorOverride) {
	UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController);
	if (!Component) {
		return;
	}

	for (int32 Index = 0; Index < StenciledActorFilterDatas.Num(); ++Index) {
		TArray<AActor*> StenciledActors;
		FindActorsInRadius(InstigatingController, InstigatingActorOverride, StenciledActorFilterDatas[Index], StenciledActors);
		if (StenciledActors.Num() > 0) {
			Component->AddActorsToStenciledList(&StenciledActors, StenciledActorFilterDatas[Index].StenciledData, bAddAsUnique, bReplaceExistingEntry, bRefreshExistingEntry);
		}
	}
}

void UFortIndicatedActorManagementLibrary::AddActorsToIndicatedList(AController* InstigatingController, const TArray<AActor*>& IndicatedActors, const FIndicatedActorData& IndicatedActorData, const bool bAddAsUnique, const bool bAllowOwningPlayer, const bool bReplaceExistingEntry, const bool bRefreshExistingEntry) {
	if (UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController)) {
		TArray<AActor*> Actors = IndicatedActors;
		Component->AddActorsToIndicatedList(&Actors, IndicatedActorData, bAddAsUnique, bAllowOwningPlayer, bReplaceExistingEntry, bRefreshExistingEntry);
	}
}

void UFortIndicatedActorManagementLibrary::AddActorsToStenciledList(AController* InstigatingController, const TArray<AActor*>& StenciledActors, const FStenciledActorData& StenciledActorData, const bool bAddAsUnique, const bool bReplaceExistingEntry, const bool bRefreshExistingEntry) {
	if (UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController)) {
		TArray<AActor*> Actors = StenciledActors;
		Component->AddActorsToStenciledList(&Actors, StenciledActorData, bAddAsUnique, bReplaceExistingEntry, bRefreshExistingEntry);
	}
}

void UFortIndicatedActorManagementLibrary::RemoveActorFromIndicatedList(AController* InstigatingController, AActor* IndicatedActor, const bool bIncludeSquad) {
	if (UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController)) {
		Component->RemoveActorFromIndicatedList(IndicatedActor, bIncludeSquad);
	}
}

void UFortIndicatedActorManagementLibrary::RemoveActorFromStenciledList(AController* InstigatingController, AActor* StenciledActor, const bool bIncludeSquad) {
	if (UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController)) {
		Component->RemoveActorFromStenciledList(StenciledActor, bIncludeSquad);
	}
}

void UFortIndicatedActorManagementLibrary::RemoveGroupFromIndicatedList(AController* InstigatingController, const FString& GroupIdentifier, const bool bIncludeSquad) {
	if (UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController)) {
		Component->RemoveGroupFromIndicatedList(GroupIdentifier, bIncludeSquad);
	}
}

void UFortIndicatedActorManagementLibrary::RemoveGroupFromStenciledList(AController* InstigatingController, const FString& GroupIdentifier, const bool bIncludeSquad) {
	if (UFortControllerComponent_IndicatedActorManagement* Component = GetIndicatedActorManagementComponent(InstigatingController)) {
		Component->RemoveGroupFromStenciledList(GroupIdentifier, bIncludeSquad);
	}
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execAddActorsInRadiusToIndicatedList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_TARRAY(FIndicatedActorDataWithFilter,Z_Param_IndicatedActorFilterDatas);
	P_GET_UBOOL(Z_Param_bAddAsUnique);
	P_GET_UBOOL(Z_Param_bReplaceExistingEntry);
	P_GET_UBOOL(Z_Param_bRefreshExistingEntry);
	P_GET_OBJECT(AActor,Z_Param_InstigatingActorOverride);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::AddActorsInRadiusToIndicatedList(Z_Param_InstigatingController,Z_Param_IndicatedActorFilterDatas,Z_Param_bAddAsUnique,Z_Param_bReplaceExistingEntry,Z_Param_bRefreshExistingEntry,Z_Param_InstigatingActorOverride);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execAddActorsInRadiusToStenciledList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_TARRAY(FIndicatedActorDataWithFilter,Z_Param_StenciledActorFilterDatas);
	P_GET_UBOOL(Z_Param_bAddAsUnique);
	P_GET_UBOOL(Z_Param_bReplaceExistingEntry);
	P_GET_UBOOL(Z_Param_bRefreshExistingEntry);
	P_GET_OBJECT(AActor,Z_Param_InstigatingActorOverride);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::AddActorsInRadiusToStenciledList(Z_Param_InstigatingController,Z_Param_StenciledActorFilterDatas,Z_Param_bAddAsUnique,Z_Param_bReplaceExistingEntry,Z_Param_bRefreshExistingEntry,Z_Param_InstigatingActorOverride);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execAddActorsToIndicatedList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_TARRAY(AActor*,Z_Param_IndicatedActors);
	P_GET_STRUCT(FIndicatedActorData,Z_Param_IndicatedActorData);
	P_GET_UBOOL(Z_Param_bAddAsUnique);
	P_GET_UBOOL(Z_Param_bAllowOwningPlayer);
	P_GET_UBOOL(Z_Param_bReplaceExistingEntry);
	P_GET_UBOOL(Z_Param_bRefreshExistingEntry);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::AddActorsToIndicatedList(Z_Param_InstigatingController,Z_Param_IndicatedActors,Z_Param_IndicatedActorData,Z_Param_bAddAsUnique,Z_Param_bAllowOwningPlayer,Z_Param_bReplaceExistingEntry,Z_Param_bRefreshExistingEntry);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execAddActorsToStenciledList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_TARRAY(AActor*,Z_Param_StenciledActors);
	P_GET_STRUCT(FStenciledActorData,Z_Param_StenciledActorData);
	P_GET_UBOOL(Z_Param_bAddAsUnique);
	P_GET_UBOOL(Z_Param_bReplaceExistingEntry);
	P_GET_UBOOL(Z_Param_bRefreshExistingEntry);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::AddActorsToStenciledList(Z_Param_InstigatingController,Z_Param_StenciledActors,Z_Param_StenciledActorData,Z_Param_bAddAsUnique,Z_Param_bReplaceExistingEntry,Z_Param_bRefreshExistingEntry);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execRemoveActorFromIndicatedList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_OBJECT(AActor,Z_Param_IndicatedActor);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::RemoveActorFromIndicatedList(Z_Param_InstigatingController,Z_Param_IndicatedActor,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execRemoveActorFromStenciledList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_OBJECT(AActor,Z_Param_StenciledActor);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::RemoveActorFromStenciledList(Z_Param_InstigatingController,Z_Param_StenciledActor,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execRemoveGroupFromIndicatedList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_PROPERTY(FStrProperty,Z_Param_GroupIdentifier);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::RemoveGroupFromIndicatedList(Z_Param_InstigatingController,Z_Param_GroupIdentifier,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortIndicatedActorManagementLibrary::execRemoveGroupFromStenciledList)
{
	P_GET_OBJECT(AController,Z_Param_InstigatingController);
	P_GET_PROPERTY(FStrProperty,Z_Param_GroupIdentifier);
	P_GET_UBOOL(Z_Param_bIncludeSquad);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortIndicatedActorManagementLibrary::RemoveGroupFromStenciledList(Z_Param_InstigatingController,Z_Param_GroupIdentifier,Z_Param_bIncludeSquad);
	P_NATIVE_END;
}

void UFortIndicatedActorManagementLibrary::Init() {
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.AddActorsInRadiusToIndicatedList"), execAddActorsInRadiusToIndicatedList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.AddActorsInRadiusToStenciledList"), execAddActorsInRadiusToStenciledList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.AddActorsToIndicatedList"), execAddActorsToIndicatedList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.AddActorsToStenciledList"), execAddActorsToStenciledList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.RemoveActorFromIndicatedList"), execRemoveActorFromIndicatedList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.RemoveActorFromStenciledList"), execRemoveActorFromStenciledList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.RemoveGroupFromIndicatedList"), execRemoveGroupFromIndicatedList);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortIndicatedActorManagementLibrary.RemoveGroupFromStenciledList"), execRemoveGroupFromStenciledList);
}

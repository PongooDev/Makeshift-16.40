#include "pch.h"

TArray<ABuildingActor*> UFortMissionLibrary::FindActorsWithTags(UObject* WorldContextObject, const FGameplayTagContainer& Tags) {
	TArray<ABuildingActor*> FoundActors;

	TArray<AActor*> BuildingActors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ABuildingActor::StaticClass(), &BuildingActors);
	for (int32 Index = 0; Index < BuildingActors.Num(); ++Index) {
		AActor* Actor = BuildingActors[Index];
		const IGameplayTagAssetInterface* TagInterface = Actor ? static_cast<const IGameplayTagAssetInterface*>(Actor->GetInterfaceAddress(IGameplayTagAssetInterface::StaticClass())) : nullptr;
		if (!TagInterface) {
			continue;
		}

		FGameplayTagContainer OwnedTags;
		TagInterface->GetOwnedGameplayTags(OwnedTags);
		if (OwnedTags.HasAll(Tags)) {
			FoundActors.Add(static_cast<ABuildingActor*>(Actor));
		}
	}

	return FoundActors;
}

DEFINE_FUNCTION(UFortMissionLibrary::execFindActorsWithTags)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_STRUCT_REF(FGameplayTagContainer,Z_Param_Out_Tags);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray<ABuildingActor*>*)Z_Param__Result=UFortMissionLibrary::FindActorsWithTags(Z_Param_WorldContextObject,Z_Param_Out_Tags);
	P_NATIVE_END;
}

void UFortMissionLibrary::Init() {
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortMissionLibrary.FindActorsWithTags"), execFindActorsWithTags);
}

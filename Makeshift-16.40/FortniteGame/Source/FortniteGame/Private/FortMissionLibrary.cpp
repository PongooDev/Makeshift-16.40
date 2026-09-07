#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/FortPickup.h"

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

static int32 GetLootWorldLevel(UWorld* World) {
	const AFortGameState* GameState = World && World->GameState ? World->GameState->Cast<AFortGameState>() : nullptr;
	return GameState ? GameState->WorldLevel : 0;
}

static AFortPickup* CreateMissionPickup(UWorld* World, const FFortItemEntry& ItemEntry, const FVector& Position, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner, const FGuid& MissionGuid) {
	FFortPickupCreationData CreationData(World, ItemEntry, Position, FRotator::ZeroRotator, OptionalOwnerPC, nullptr, nullptr);
	CreationData.bPickupOnlyRelevantToOwner = bPickupOnlyRelevantToOwner;

	AFortPickup* Pickup = AFortPickup::CreateFromData(CreationData);
	if (Pickup && Pickup->Role == ENetRole::ROLE_Authority) {
		Pickup->OptionalMissionGuid = MissionGuid;
	}

	return Pickup;
}

AFortPickup* UFortMissionLibrary::SpawnMissionItemPickupInWorld(UObject* WorldContextObject, const FGuid& MissionGuid, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount) {
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !ItemDefinition || NumberToSpawn <= 0) {
		return nullptr;
	}

	FFortItemEntry ItemEntry(ItemDefinition, NumberToSpawn, 0);
	AFortPickup* Pickup = CreateMissionPickup(World, ItemEntry, Position, nullptr, false, MissionGuid);
	if (Pickup) {
		Pickup->TossPickup(Position + Direction, nullptr, OverrideMaxStackCount, true, true, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset);
	}

	return Pickup;
}

void UFortMissionLibrary::GiveMissionRewardsToPlayerAsPickups(UObject* WorldContextObject, const FGuid& MissionGuid, AFortPlayerController* PlayerController, const FVector& StartPosition, float Radius, FName LootTierGroup, int32 LootTier) {
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || LootTierGroup == FName()) {
		return;
	}

	TArray<AFortPlayerController*> PlayerControllers;
	if (PlayerController) {
		PlayerControllers.Add(PlayerController);
	}
	else {
		PlayerControllers = UFortKismetLibrary::GetAllFortPlayerControllers(WorldContextObject, true, true);
	}

	const int32 WorldLevel = GetLootWorldLevel(World);
	for (int32 PlayerIndex = 0; PlayerIndex < PlayerControllers.Num(); ++PlayerIndex) {
		AFortPlayerController* RewardedPlayer = PlayerControllers[PlayerIndex];
		if (!RewardedPlayer) {
			continue;
		}

		TArray<FFortItemEntry> LootToDrop;
		if (!UFortKismetLibrary::PickLootDrops(WorldContextObject, LootToDrop, LootTierGroup, WorldLevel, LootTier)) {
			continue;
		}

		for (int32 LootIndex = 0; LootIndex < LootToDrop.Num(); ++LootIndex) {
			FVector Offset = FMath::VRand();
			Offset.Z = 0.f;
			const FVector SpawnPosition = StartPosition + Offset * FMath::FRandRange(0.f, Radius);

			AFortPickup* Pickup = CreateMissionPickup(World, LootToDrop[LootIndex], SpawnPosition, RewardedPlayer, true, MissionGuid);
			if (Pickup) {
				Pickup->TossPickup(SpawnPosition + FMath::VRand(), nullptr, 0, true, true, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset);
			}
		}
	}
}

bool UFortMissionLibrary::GiveMissionSchematicItemDirectlyToPlayer(UObject* WorldContextObject, AFortPlayerController* TargetPlayer, const FGuid& MissionGuid, UFortSchematicItemDefinition* SchematicDefinition) {
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !TargetPlayer || !SchematicDefinition) {
		return false;
	}

	UE_LOG(LogFort, Warning, TEXT("GiveMissionSchematicItemDirectlyToPlayer: no profile service to grant %hs to %hs for mission %s"), SchematicDefinition->GetName().c_str(), TargetPlayer->GetName().c_str(), *MissionGuid.ToString());
	return false;
}

void UFortMissionLibrary::SpawnAndCollectPlayerPickups(UObject* WorldContextObject, AFortPlayerController* PlayerController, const FVector& StartPosition, FName LootTierGroup, int32 LootTier) {
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || !PlayerController || LootTierGroup == FName()) {
		return;
	}

	TArray<FFortItemEntry> LootToDrop;
	if (!UFortKismetLibrary::PickLootDrops(WorldContextObject, LootToDrop, LootTierGroup, GetLootWorldLevel(World), LootTier)) {
		return;
	}

	AFortPawn* PlayerPawn = PlayerController->Pawn ? PlayerController->Pawn->Cast<AFortPawn>() : nullptr;
	FVector StartDirection = FVector();
	StartDirection.Z = 1.f;
	for (int32 LootIndex = 0; LootIndex < LootToDrop.Num(); ++LootIndex) {
		AFortPickup* Pickup = CreateMissionPickup(World, LootToDrop[LootIndex], StartPosition, PlayerController, true, FGuid());
		if (!Pickup) {
			continue;
		}

		if (PlayerPawn) {
			Pickup->SetPickupTarget(PlayerPawn, 1.f, StartDirection, true);
		}
		else {
			Pickup->TossPickup(StartPosition + FMath::VRand(), nullptr, 0, true, true, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset);
		}
	}
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

DEFINE_FUNCTION(UFortMissionLibrary::execSpawnMissionItemPickupInWorld)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_STRUCT(FGuid,Z_Param_MissionGuid);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToSpawn);
	P_GET_STRUCT(FVector,Z_Param_Position);
	P_GET_STRUCT(FVector,Z_Param_Direction);
	P_GET_PROPERTY(FIntProperty,Z_Param_OverrideMaxStackCount);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(AFortPickup**)Z_Param__Result=UFortMissionLibrary::SpawnMissionItemPickupInWorld(Z_Param_WorldContextObject,Z_Param_MissionGuid,Z_Param_ItemDefinition,Z_Param_NumberToSpawn,Z_Param_Position,Z_Param_Direction,Z_Param_OverrideMaxStackCount);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortMissionLibrary::execGiveMissionRewardsToPlayerAsPickups)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_STRUCT(FGuid,Z_Param_MissionGuid);
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_STRUCT(FVector,Z_Param_StartPosition);
	P_GET_PROPERTY(FFloatProperty,Z_Param_Radius);
	P_GET_PROPERTY(FNameProperty,Z_Param_LootTierGroup);
	P_GET_PROPERTY(FIntProperty,Z_Param_LootTier);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortMissionLibrary::GiveMissionRewardsToPlayerAsPickups(Z_Param_WorldContextObject,Z_Param_MissionGuid,Z_Param_PlayerController,Z_Param_StartPosition,Z_Param_Radius,Z_Param_LootTierGroup,Z_Param_LootTier);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortMissionLibrary::execGiveMissionSchematicItemDirectlyToPlayer)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(AFortPlayerController,Z_Param_TargetPlayer);
	P_GET_STRUCT(FGuid,Z_Param_MissionGuid);
	P_GET_OBJECT(UFortSchematicItemDefinition,Z_Param_SchematicDefinition);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UFortMissionLibrary::GiveMissionSchematicItemDirectlyToPlayer(Z_Param_WorldContextObject,Z_Param_TargetPlayer,Z_Param_MissionGuid,Z_Param_SchematicDefinition);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortMissionLibrary::execSpawnAndCollectPlayerPickups)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_STRUCT(FVector,Z_Param_StartPosition);
	P_GET_PROPERTY(FNameProperty,Z_Param_LootTierGroup);
	P_GET_PROPERTY(FIntProperty,Z_Param_LootTier);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortMissionLibrary::SpawnAndCollectPlayerPickups(Z_Param_WorldContextObject,Z_Param_PlayerController,Z_Param_StartPosition,Z_Param_LootTierGroup,Z_Param_LootTier);
	P_NATIVE_END;
}

void UFortMissionLibrary::Init() {
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortMissionLibrary.FindActorsWithTags"), execFindActorsWithTags);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortMissionLibrary.GiveMissionRewardsToPlayerAsPickups"), execGiveMissionRewardsToPlayerAsPickups);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortMissionLibrary.GiveMissionSchematicItemDirectlyToPlayer"), execGiveMissionSchematicItemDirectlyToPlayer);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortMissionLibrary.SpawnAndCollectPlayerPickups"), execSpawnAndCollectPlayerPickups);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortMissionLibrary.SpawnMissionItemPickupInWorld"), execSpawnMissionItemPickupInWorld);
}

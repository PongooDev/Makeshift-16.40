#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"
#include "FortniteGame/Source/FortniteGame/Public/FortPickup.h"
#include "FortniteGame/Source/FortniteGame/Public/FortAssets.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/DataTable.h"

int32 UFortKismetLibrary::IncrementAnalyticMatchCount(const UObject* WorldContextObject, const EAnalyticMatchCounts MatchCountID, const int32 AmountToAdd) {
	static const UWorld* AnalyticMatchCountsWorld = nullptr;
	static int32 AnalyticMatchCounts[static_cast<int32>(EAnalyticMatchCounts::Count)] = {};

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || MatchCountID >= EAnalyticMatchCounts::Count) {
		return 0;
	}

	if (AnalyticMatchCountsWorld != World) {
		AnalyticMatchCountsWorld = World;
		FMemory::Memzero(AnalyticMatchCounts, sizeof(AnalyticMatchCounts));
	}

	AnalyticMatchCounts[static_cast<int32>(MatchCountID)] += AmountToAdd;
	return AnalyticMatchCounts[static_cast<int32>(MatchCountID)];
}

DEFINE_FUNCTION(UFortKismetLibrary::execIncrementAnalyticMatchCount)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_ENUM(EAnalyticMatchCounts,Z_Param_MatchCountID);
	P_GET_PROPERTY(FIntProperty,Z_Param_AmountToAdd);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=UFortKismetLibrary::IncrementAnalyticMatchCount(Z_Param_WorldContextObject,EAnalyticMatchCounts(Z_Param_MatchCountID),Z_Param_AmountToAdd);
	P_NATIVE_END;
}

AFortPlayerController* UFortKismetLibrary::GetFortPlayerControllerFromActor(AActor* Actor) {
	AFortPlayerController* (*Fn)(AActor*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x1F9B498);
	return Fn(Actor);
}

void UFortKismetLibrary::ChangeTeam(AActor* PlayerToSwitch, AActor* Instigator, uint8 NewTeam, const FGameplayTagContainer& ChangeTeamTags) {
	AFortPlayerController* PlayerController = GetFortPlayerControllerFromActor(PlayerToSwitch);
	AFortPlayerControllerAthena* PlayerControllerAthena = PlayerController ? PlayerController->Cast<AFortPlayerControllerAthena>() : nullptr;
	if (!PlayerControllerAthena) {
		return;
	}

	AFortPlayerStateAthena* PlayerState = PlayerControllerAthena->PlayerState ? PlayerControllerAthena->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
	if (PlayerState) {
		PlayerState->ChangeTeamInfo.Instigator = Instigator;
		PlayerState->ChangeTeamInfo.ChangeTeamTags = ChangeTeamTags;
	}

	PlayerControllerAthena->ServerSetTeam_Implementation(NewTeam);
}

DEFINE_FUNCTION(UFortKismetLibrary::execChangeTeam)
{
	P_GET_OBJECT(AActor,Z_Param_PlayerToSwitch);
	P_GET_OBJECT(AActor,Z_Param_Instigator);
	P_GET_PROPERTY(FByteProperty,Z_Param_NewTeam);
	P_GET_STRUCT_REF(FGameplayTagContainer,Z_Param_Out_ChangeTeamTags);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortKismetLibrary::ChangeTeam(Z_Param_PlayerToSwitch,Z_Param_Instigator,Z_Param_NewTeam,Z_Param_Out_ChangeTeamTags);
	P_NATIVE_END;
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldWithClassAndItemEntry(UObject* WorldContextObject, const FFortItemEntry& ItemEntry, TSubclassOf<AFortPickup> PickupClass, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner, bool bShouldCombinePickupsWhenTossCompletes) {
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) {
		return nullptr;
	}

	FFortPickupCreationData CreationData(World, ItemEntry, Position, FRotator::ZeroRotator, OptionalOwnerPC, PickupClass, nullptr);
	CreationData.SourceTypeFlags = SourceType;
	CreationData.SpawnSource = Source;
	CreationData.bRandomRotation = bRandomRotation;
	CreationData.bPickupOnlyRelevantToOwner = bPickupOnlyRelevantToOwner;

	AFortPickup* Pickup = AFortPickup::CreateFromData(CreationData);
	if (Pickup) {
		Pickup->bBlockedFromAutoPickup = bBlockedFromAutoPickup;
		Pickup->TossPickup(bToss ? Position + Direction : Position, nullptr, OverrideMaxStackCount, bToss, bShouldCombinePickupsWhenTossCompletes, SourceType, Source);
	}

	return Pickup;
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldWithClassAndLevel(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 WorldLevel, TSubclassOf<AFortPickup> PickupClass, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, int32 PickupInstigatorHandle, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner) {
	if (!ItemDefinition || NumberToSpawn <= 0) {
		return nullptr;
	}

	FFortItemEntry ItemEntry(ItemDefinition, NumberToSpawn, WorldLevel);
	if (PickupInstigatorHandle != INDEX_NONE) {
		ItemEntry.SetStateValue(EFortItemEntryState::PickupInstigatorHandle, PickupInstigatorHandle);
	}

	return K2_SpawnPickupInWorldWithClassAndItemEntry(WorldContextObject, ItemEntry, PickupClass, Position, Direction, OverrideMaxStackCount, bToss, bRandomRotation, bBlockedFromAutoPickup, SourceType, Source, OptionalOwnerPC, bPickupOnlyRelevantToOwner);
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, int32 PickupInstigatorHandle, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner) {
	return K2_SpawnPickupInWorldWithClassAndLevel(WorldContextObject, ItemDefinition, 0, nullptr, NumberToSpawn, Position, Direction, OverrideMaxStackCount, bToss, bRandomRotation, bBlockedFromAutoPickup, PickupInstigatorHandle, SourceType, Source, OptionalOwnerPC, bPickupOnlyRelevantToOwner);
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldWithClass(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, TSubclassOf<AFortPickup> PickupClass, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, int32 PickupInstigatorHandle, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner) {
	return K2_SpawnPickupInWorldWithClassAndLevel(WorldContextObject, ItemDefinition, 0, PickupClass, NumberToSpawn, Position, Direction, OverrideMaxStackCount, bToss, bRandomRotation, bBlockedFromAutoPickup, PickupInstigatorHandle, SourceType, Source, OptionalOwnerPC, bPickupOnlyRelevantToOwner);
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldWithLevel(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 WorldLevel, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, int32 PickupInstigatorHandle, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner) {
	return K2_SpawnPickupInWorldWithClassAndLevel(WorldContextObject, ItemDefinition, WorldLevel, nullptr, NumberToSpawn, Position, Direction, OverrideMaxStackCount, bToss, bRandomRotation, bBlockedFromAutoPickup, PickupInstigatorHandle, SourceType, Source, OptionalOwnerPC, bPickupOnlyRelevantToOwner);
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_SpawnPickupInWorld)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToSpawn);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Position);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Direction);
	P_GET_PROPERTY(FIntProperty,Z_Param_OverrideMaxStackCount);
	P_GET_UBOOL(Z_Param_bToss);
	P_GET_UBOOL(Z_Param_bRandomRotation);
	P_GET_UBOOL(Z_Param_bBlockedFromAutoPickup);
	P_GET_PROPERTY(FIntProperty,Z_Param_PickupInstigatorHandle);
	P_GET_ENUM(EFortPickupSourceTypeFlag,Z_Param_SourceType);
	P_GET_ENUM(EFortPickupSpawnSource,Z_Param_Source);
	P_GET_OBJECT(AFortPlayerController,Z_Param_OptionalOwnerPC);
	P_GET_UBOOL(Z_Param_bPickupOnlyRelevantToOwner);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(AFortPickup**)Z_Param__Result=UFortKismetLibrary::K2_SpawnPickupInWorld(Z_Param_WorldContextObject,Z_Param_ItemDefinition,Z_Param_NumberToSpawn,Z_Param_Out_Position,Z_Param_Out_Direction,Z_Param_OverrideMaxStackCount,Z_Param_bToss,Z_Param_bRandomRotation,Z_Param_bBlockedFromAutoPickup,Z_Param_PickupInstigatorHandle,EFortPickupSourceTypeFlag(Z_Param_SourceType),EFortPickupSpawnSource(Z_Param_Source),Z_Param_OptionalOwnerPC,Z_Param_bPickupOnlyRelevantToOwner);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_SpawnPickupInWorldWithClass)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_OBJECT(UClass,Z_Param_PickupClass);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToSpawn);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Position);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Direction);
	P_GET_PROPERTY(FIntProperty,Z_Param_OverrideMaxStackCount);
	P_GET_UBOOL(Z_Param_bToss);
	P_GET_UBOOL(Z_Param_bRandomRotation);
	P_GET_UBOOL(Z_Param_bBlockedFromAutoPickup);
	P_GET_PROPERTY(FIntProperty,Z_Param_PickupInstigatorHandle);
	P_GET_ENUM(EFortPickupSourceTypeFlag,Z_Param_SourceType);
	P_GET_ENUM(EFortPickupSpawnSource,Z_Param_Source);
	P_GET_OBJECT(AFortPlayerController,Z_Param_OptionalOwnerPC);
	P_GET_UBOOL(Z_Param_bPickupOnlyRelevantToOwner);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(AFortPickup**)Z_Param__Result=UFortKismetLibrary::K2_SpawnPickupInWorldWithClass(Z_Param_WorldContextObject,Z_Param_ItemDefinition,Z_Param_PickupClass,Z_Param_NumberToSpawn,Z_Param_Out_Position,Z_Param_Out_Direction,Z_Param_OverrideMaxStackCount,Z_Param_bToss,Z_Param_bRandomRotation,Z_Param_bBlockedFromAutoPickup,Z_Param_PickupInstigatorHandle,EFortPickupSourceTypeFlag(Z_Param_SourceType),EFortPickupSpawnSource(Z_Param_Source),Z_Param_OptionalOwnerPC,Z_Param_bPickupOnlyRelevantToOwner);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_SpawnPickupInWorldWithClassAndLevel)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_WorldLevel);
	P_GET_OBJECT(UClass,Z_Param_PickupClass);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToSpawn);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Position);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Direction);
	P_GET_PROPERTY(FIntProperty,Z_Param_OverrideMaxStackCount);
	P_GET_UBOOL(Z_Param_bToss);
	P_GET_UBOOL(Z_Param_bRandomRotation);
	P_GET_UBOOL(Z_Param_bBlockedFromAutoPickup);
	P_GET_PROPERTY(FIntProperty,Z_Param_PickupInstigatorHandle);
	P_GET_ENUM(EFortPickupSourceTypeFlag,Z_Param_SourceType);
	P_GET_ENUM(EFortPickupSpawnSource,Z_Param_Source);
	P_GET_OBJECT(AFortPlayerController,Z_Param_OptionalOwnerPC);
	P_GET_UBOOL(Z_Param_bPickupOnlyRelevantToOwner);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(AFortPickup**)Z_Param__Result=UFortKismetLibrary::K2_SpawnPickupInWorldWithClassAndLevel(Z_Param_WorldContextObject,Z_Param_ItemDefinition,Z_Param_WorldLevel,Z_Param_PickupClass,Z_Param_NumberToSpawn,Z_Param_Out_Position,Z_Param_Out_Direction,Z_Param_OverrideMaxStackCount,Z_Param_bToss,Z_Param_bRandomRotation,Z_Param_bBlockedFromAutoPickup,Z_Param_PickupInstigatorHandle,EFortPickupSourceTypeFlag(Z_Param_SourceType),EFortPickupSpawnSource(Z_Param_Source),Z_Param_OptionalOwnerPC,Z_Param_bPickupOnlyRelevantToOwner);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_SpawnPickupInWorldWithLevel)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_WorldLevel);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToSpawn);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Position);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Direction);
	P_GET_PROPERTY(FIntProperty,Z_Param_OverrideMaxStackCount);
	P_GET_UBOOL(Z_Param_bToss);
	P_GET_UBOOL(Z_Param_bRandomRotation);
	P_GET_UBOOL(Z_Param_bBlockedFromAutoPickup);
	P_GET_PROPERTY(FIntProperty,Z_Param_PickupInstigatorHandle);
	P_GET_ENUM(EFortPickupSourceTypeFlag,Z_Param_SourceType);
	P_GET_ENUM(EFortPickupSpawnSource,Z_Param_Source);
	P_GET_OBJECT(AFortPlayerController,Z_Param_OptionalOwnerPC);
	P_GET_UBOOL(Z_Param_bPickupOnlyRelevantToOwner);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(AFortPickup**)Z_Param__Result=UFortKismetLibrary::K2_SpawnPickupInWorldWithLevel(Z_Param_WorldContextObject,Z_Param_ItemDefinition,Z_Param_WorldLevel,Z_Param_NumberToSpawn,Z_Param_Out_Position,Z_Param_Out_Direction,Z_Param_OverrideMaxStackCount,Z_Param_bToss,Z_Param_bRandomRotation,Z_Param_bBlockedFromAutoPickup,Z_Param_PickupInstigatorHandle,EFortPickupSourceTypeFlag(Z_Param_SourceType),EFortPickupSpawnSource(Z_Param_Source),Z_Param_OptionalOwnerPC,Z_Param_bPickupOnlyRelevantToOwner);
	P_NATIVE_END;
}

bool UFortKismetLibrary::SpawnInstancedPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup) {
	bool bSpawnedPickup = false;

	TArray<AFortPlayerController*> PlayerControllers = GetAllFortPlayerControllers(WorldContextObject, true, false);
	for (int32 Index = 0; Index < PlayerControllers.Num(); ++Index) {
		if (K2_SpawnPickupInWorldWithClassAndLevel(WorldContextObject, ItemDefinition, 0, nullptr, NumberToSpawn, Position, Direction, OverrideMaxStackCount, bToss, bRandomRotation, bBlockedFromAutoPickup, INDEX_NONE, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, PlayerControllers[Index], true)) {
			bSpawnedPickup = true;
		}
	}

	return bSpawnedPickup;
}

DEFINE_FUNCTION(UFortKismetLibrary::execSpawnInstancedPickupInWorld)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToSpawn);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Position);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Direction);
	P_GET_PROPERTY(FIntProperty,Z_Param_OverrideMaxStackCount);
	P_GET_UBOOL(Z_Param_bToss);
	P_GET_UBOOL(Z_Param_bRandomRotation);
	P_GET_UBOOL(Z_Param_bBlockedFromAutoPickup);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UFortKismetLibrary::SpawnInstancedPickupInWorld(Z_Param_WorldContextObject,Z_Param_ItemDefinition,Z_Param_NumberToSpawn,Z_Param_Out_Position,Z_Param_Out_Direction,Z_Param_OverrideMaxStackCount,Z_Param_bToss,Z_Param_bRandomRotation,Z_Param_bBlockedFromAutoPickup);
	P_NATIVE_END;
}

static int32 PickItemVariantIndex(const UFortItemVariantData& ItemVariantData, const FGameplayTagContainer& RequiredTags) {
	TArray<int32> EligibleVariantIndices;
	TArray<float> EligibleVariantWeights;
	float TotalWeight = 0.f;
	for (int32 VariantIndex = 0; VariantIndex < ItemVariantData.Variants.Num(); ++VariantIndex) {
		const FFortVariantData& Variant = ItemVariantData.Variants[VariantIndex];
		if (RequiredTags.HasAll(Variant.RequiredTags)) {
			const float Weight = FMath::Max(Variant.Weight.GetValueAtLevel(0.f), 0.f);
			EligibleVariantIndices.Add(VariantIndex);
			EligibleVariantWeights.Add(Weight);
			TotalWeight += Weight;
		}
	}

	if (EligibleVariantIndices.Num() == 0) {
		return INDEX_NONE;
	}

	if (TotalWeight <= 0.f) {
		return EligibleVariantIndices[FMath::RandRange(0, EligibleVariantIndices.Num() - 1)];
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (int32 EligibleIndex = 0; EligibleIndex < EligibleVariantIndices.Num(); ++EligibleIndex) {
		Roll -= EligibleVariantWeights[EligibleIndex];
		if (Roll <= 0.f) {
			return EligibleVariantIndices[EligibleIndex];
		}
	}

	return EligibleVariantIndices[EligibleVariantIndices.Num() - 1];
}

AFortPickup* UFortKismetLibrary::SpawnItemVariantPickupInWorld(UObject* WorldContextObject, const FSpawnItemVariantParams& Params) {
	if (!Params.WorldItemDefinition || Params.NumberToSpawn <= 0) {
		return nullptr;
	}

	FFortItemEntry ItemEntry(Params.WorldItemDefinition, Params.NumberToSpawn, 0);
	if (Params.PickupInstigatorHandle != INDEX_NONE) {
		ItemEntry.SetStateValue(EFortItemEntryState::PickupInstigatorHandle, Params.PickupInstigatorHandle);
	}

	FGameplayTag ItemVariantTag{};
	int32 ItemVariantDataMappingIndex = INDEX_NONE;
	const UGameDataBR* GameDataBR = UGameDataBR::Get();
	const UFortItemVariantData* ItemVariantData = GameDataBR ? GameDataBR->GetItemVariantDataFromItemDefTags(Params.WorldItemDefinition->GameplayTags, ItemVariantTag, ItemVariantDataMappingIndex) : nullptr;
	if (ItemVariantData) {
		const int32 PickupVariantIndex = PickItemVariantIndex(*ItemVariantData, Params.RequiredTags);
		if (PickupVariantIndex != INDEX_NONE) {
			ItemEntry.SetPickupVariantIndex(PickupVariantIndex);
			ItemEntry.SetItemVariantDataMappingIndex(ItemVariantDataMappingIndex);
		}
	}

	return K2_SpawnPickupInWorldWithClassAndItemEntry(WorldContextObject, ItemEntry, nullptr, Params.Position, Params.Direction, Params.OverrideMaxStackCount, Params.bToss, Params.bRandomRotation, Params.bBlockedFromAutoPickup, Params.SourceType, Params.Source, Params.OptionalOwnerPC, Params.bPickupOnlyRelevantToOwner);
}

DEFINE_FUNCTION(UFortKismetLibrary::execSpawnItemVariantPickupInWorld)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_STRUCT(FSpawnItemVariantParams,Z_Param_Params_0);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(AFortPickup**)Z_Param__Result=UFortKismetLibrary::SpawnItemVariantPickupInWorld(Z_Param_WorldContextObject,Z_Param_Params_0);
	P_NATIVE_END;
}

static const int32 MaxLootPackageCallDepth = 16;

static void GetLootTierAndPackageTables(UWorld* World, TArray<UDataTable*>& OutLootTierTables, TArray<UDataTable*>& OutLootPackageTables) {
	AFortGameStateAthena* GameState = World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
	const UFortPlaylistAthena* Playlist = GameState ? (GameState->CurrentPlaylistInfo.OverridePlaylist ? GameState->CurrentPlaylistInfo.OverridePlaylist : GameState->CurrentPlaylistInfo.BasePlaylist) : nullptr;

	UDataTable* LootTierTable = Playlist ? FFortAssets::GetAsset(Playlist->LootTierData) : nullptr;
	if (LootTierTable) {
		OutLootTierTables.Add(LootTierTable);
	} else if (const UGameDataBR* GameDataBR = UGameDataBR::Get()) {
		GameDataBR->GetLootTierDataTablesBR(OutLootTierTables);
	}

	UDataTable* LootPackageTable = Playlist ? FFortAssets::GetAsset(Playlist->LootPackages) : nullptr;
	if (LootPackageTable) {
		OutLootPackageTables.Add(LootPackageTable);
	} else if (const UGameDataBR* GameDataBR = UGameDataBR::Get()) {
		GameDataBR->GetLootPackageDataTablesBR(OutLootPackageTables);
	}

	AFortGameModeAthena* GameMode = World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameModeAthena>() : nullptr;
	if (GameMode) {
		FFortAssets::GetAssetArray(GameMode->GameFeatureLootTierData, OutLootTierTables);
		FFortAssets::GetAssetArray(GameMode->GameFeatureLootPackageData, OutLootPackageTables);
	}
}

template<typename RowType>
static const RowType* PickWeightedLootRow(const TArray<const RowType*>& Candidates, const TArray<float>& Weights, const float TotalWeight) {
	if (Candidates.Num() == 0 || TotalWeight <= 0.f) {
		return nullptr;
	}

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (int32 Index = 0; Index < Candidates.Num(); ++Index) {
		Roll -= Weights[Index];
		if (Roll <= 0.f) {
			return Candidates[Index];
		}
	}

	return Candidates[Candidates.Num() - 1];
}

static bool IsWorldLevelInRange(const int32 WorldLevel, const int32 MinWorldLevel, const int32 MaxWorldLevel) {
	return (MinWorldLevel < 0 || WorldLevel >= MinWorldLevel) && (MaxWorldLevel < 0 || WorldLevel <= MaxWorldLevel);
}

static const FFortLootTierData* PickLootTierData(UWorld* World, const TArray<UDataTable*>& LootTierTables, const FName TierGroupName, const int32 WorldLevel, const int32 ForcedLootTier, const TMap<FName, float>* NamedWeightsMap, const FGameplayTagContainer& PlaylistContextTags) {
	TArray<const FFortLootTierData*> Candidates;
	TArray<float> Weights;
	float TotalWeight = 0.f;
	for (UDataTable* LootTierTable : LootTierTables) {
		if (!LootTierTable) {
			continue;
		}

		TArray<FFortLootTierData*> LootTierRows;
		LootTierTable->GetAllRows<FFortLootTierData>(TEXT("UFortKismetLibrary::PickLootDrops LootTierTable"), LootTierRows);
		for (const FFortLootTierData* LootTierData : LootTierRows) {
			if (!LootTierData || LootTierData->TierGroup != TierGroupName) {
				continue;
			}
			if (ForcedLootTier >= 0 && LootTierData->LootTier != ForcedLootTier) {
				continue;
			}
			if (!IsWorldLevelInRange(WorldLevel, LootTierData->MinWorldLevel, LootTierData->MaxWorldLevel) || !PlaylistContextTags.HasAll(LootTierData->RequiredGameplayTags)) {
				continue;
			}

			const float LootTierWeight = LootTierData->GetTrueWeight(World, NamedWeightsMap);
			if (LootTierWeight <= 0.f) {
				continue;
			}

			Candidates.Add(LootTierData);
			Weights.Add(LootTierWeight);
			TotalWeight += LootTierWeight;
		}
	}

	return PickWeightedLootRow(Candidates, Weights, TotalWeight);
}

static const FFortLootPackageData* PickLootPackageData(UWorld* World, const TArray<UDataTable*>& LootPackageTables, const FName LootPackageID, const int32 LootPackageCategory, const int32 WorldLevel, const TMap<FName, float>* NamedWeightsMap) {
	TArray<const FFortLootPackageData*> Candidates;
	TArray<float> Weights;
	float TotalWeight = 0.f;
	for (UDataTable* LootPackageTable : LootPackageTables) {
		if (!LootPackageTable) {
			continue;
		}

		TArray<FFortLootPackageData*> LootPackageRows;
		LootPackageTable->GetAllRows<FFortLootPackageData>(TEXT("UFortKismetLibrary::PickLootDrops LootPackageTable"), LootPackageRows);
		for (const FFortLootPackageData* LootPackageData : LootPackageRows) {
			if (!LootPackageData || LootPackageData->LootPackageID != LootPackageID || LootPackageData->LootPackageCategory != LootPackageCategory) {
				continue;
			}
			if (!IsWorldLevelInRange(WorldLevel, LootPackageData->MinWorldLevel, LootPackageData->MaxWorldLevel)) {
				continue;
			}

			const float LootPackageWeight = LootPackageData->GetTrueWeight(World, NamedWeightsMap);
			if (LootPackageWeight <= 0.f || LootPackageData->GetTrueCount(World) <= 0.f) {
				continue;
			}

			Candidates.Add(LootPackageData);
			Weights.Add(LootPackageWeight);
			TotalWeight += LootPackageWeight;
		}
	}

	return PickWeightedLootRow(Candidates, Weights, TotalWeight);
}

static void PickLootPackageDrops(UWorld* World, const TArray<UDataTable*>& LootPackageTables, const FName LootPackageID, const int32 LootPackageCategory, const int32 WorldLevel, const TMap<FName, float>* NamedWeightsMap, TArray<FFortItemEntry>& OutLootToDrop, const int32 LootPackageCallDepth) {
	const FFortLootPackageData* LootPackageData = PickLootPackageData(World, LootPackageTables, LootPackageID, LootPackageCategory, WorldLevel, NamedWeightsMap);
	if (!LootPackageData) {
		return;
	}

	if (LootPackageData->LootPackageCall.Num() > 1) {
		if (LootPackageCallDepth < MaxLootPackageCallDepth) {
			PickLootPackageDrops(World, LootPackageTables, FName(*LootPackageData->LootPackageCall), 0, WorldLevel, NamedWeightsMap, OutLootToDrop, LootPackageCallDepth + 1);
		}
		return;
	}

	UFortItemDefinition* ItemDefinition = FFortAssets::GetAsset(LootPackageData->ItemDefinition);
	if (!ItemDefinition) {
		return;
	}

	const int32 Count = FMath::TruncToInt(LootPackageData->GetTrueCount(World));
	if (Count <= 0) {
		return;
	}

	OutLootToDrop.Add(FFortItemEntry(ItemDefinition, Count, WorldLevel));
}

bool UFortKismetLibrary::PickLootDropsWithNamedWeights(UObject* WorldContextObject, TArray<FFortItemEntry>& OutLootToDrop, const FName TierGroupName, const int32 WorldLevel, const TMap<FName, float>& NamedWeightsMap, const int32 ForcedLootTier) {
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) {
		return false;
	}

	TArray<UDataTable*> LootTierTables;
	TArray<UDataTable*> LootPackageTables;
	GetLootTierAndPackageTables(World, LootTierTables, LootPackageTables);

	FName LootTierGroup = TierGroupName;
	AFortGameModeAthena* GameMode = World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameModeAthena>() : nullptr;
	if (GameMode) {
		if (const FName* RedirectedLootTierGroup = GameMode->RedirectAthenaLootTierGroups.Find(LootTierGroup)) {
			LootTierGroup = *RedirectedLootTierGroup;
		}
	}

	AFortGameStateAthena* GameState = World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
	const FGameplayTagContainer PlaylistContextTags = GameState ? GameState->GetAthenaPlaylistContextTags() : FGameplayTagContainer();

	const FFortLootTierData* LootTierData = PickLootTierData(World, LootTierTables, LootTierGroup, WorldLevel, ForcedLootTier, &NamedWeightsMap, PlaylistContextTags);
	if (!LootTierData) {
		return false;
	}

	const int32 InitialLootNum = OutLootToDrop.Num();
	const int32 NumLootPackageDrops = FMath::FloorToInt(LootTierData->NumLootPackageDrops) + (FMath::FRand() < FMath::Fractional(LootTierData->NumLootPackageDrops) ? 1 : 0);
	const int32 NumLootPackageCategories = FMath::Min(LootTierData->LootPackageCategoryWeightArray.Num(), FMath::Min(LootTierData->LootPackageCategoryMinArray.Num(), LootTierData->LootPackageCategoryMaxArray.Num()));
	if (NumLootPackageCategories == 0) {
		for (int32 DropIndex = 0; DropIndex < NumLootPackageDrops; ++DropIndex) {
			PickLootPackageDrops(World, LootPackageTables, LootTierData->LootPackage, 0, WorldLevel, &NamedWeightsMap, OutLootToDrop, 0);
		}
		return OutLootToDrop.Num() > InitialLootNum;
	}

	TArray<int32> CategoryDrops;
	int32 NumDrops = 0;
	for (int32 Category = 0; Category < NumLootPackageCategories; ++Category) {
		CategoryDrops.Add(0);
		for (int32 MinIndex = 0; MinIndex < LootTierData->LootPackageCategoryMinArray[Category]; ++MinIndex) {
			PickLootPackageDrops(World, LootPackageTables, LootTierData->LootPackage, Category, WorldLevel, &NamedWeightsMap, OutLootToDrop, 0);
			++CategoryDrops[Category];
			++NumDrops;
		}
	}

	while (NumDrops < NumLootPackageDrops) {
		int32 TotalCategoryWeight = 0;
		for (int32 Category = 0; Category < NumLootPackageCategories; ++Category) {
			if (LootTierData->LootPackageCategoryMaxArray[Category] < 0 || CategoryDrops[Category] < LootTierData->LootPackageCategoryMaxArray[Category]) {
				TotalCategoryWeight += FMath::Max(LootTierData->LootPackageCategoryWeightArray[Category], 0);
			}
		}
		if (TotalCategoryWeight <= 0) {
			break;
		}

		int32 Roll = FMath::RandRange(0, TotalCategoryWeight - 1);
		for (int32 Category = 0; Category < NumLootPackageCategories; ++Category) {
			if (LootTierData->LootPackageCategoryMaxArray[Category] >= 0 && CategoryDrops[Category] >= LootTierData->LootPackageCategoryMaxArray[Category]) {
				continue;
			}
			Roll -= FMath::Max(LootTierData->LootPackageCategoryWeightArray[Category], 0);
			if (Roll < 0) {
				PickLootPackageDrops(World, LootPackageTables, LootTierData->LootPackage, Category, WorldLevel, &NamedWeightsMap, OutLootToDrop, 0);
				++CategoryDrops[Category];
				break;
			}
		}
		++NumDrops;
	}

	return OutLootToDrop.Num() > InitialLootNum;
}

bool UFortKismetLibrary::PickLootDrops(UObject* WorldContextObject, TArray<FFortItemEntry>& OutLootToDrop, const FName TierGroupName, const int32 WorldLevel, const int32 ForcedLootTier) {
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	AFortGameModeZone* GameMode = World && World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameModeZone>() : nullptr;
	return PickLootDropsWithNamedWeights(WorldContextObject, OutLootToDrop, TierGroupName, WorldLevel, GameMode ? GameMode->ZoneLootNamedWeightsMapOverride : TMap<FName, float>(), ForcedLootTier);
}

static int32 GetLootWorldLevel(UWorld* World) {
	const AFortGameState* GameState = World && World->GameState ? World->GameState->Cast<AFortGameState>() : nullptr;
	return GameState ? GameState->WorldLevel : 0;
}

TArray<AFortPickup*> UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTier(UObject* WorldContextObject, FName LootTierName, const FVector& Position, int32 OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source) {
	TArray<AFortPickup*> Pickups;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	TArray<FFortItemEntry> LootToDrop;
	if (World && PickLootDrops(WorldContextObject, LootToDrop, LootTierName, GetLootWorldLevel(World), INDEX_NONE)) {
		const FVector Direction = FVector();
		for (const FFortItemEntry& ItemEntry : LootToDrop) {
			AFortPickup* Pickup = K2_SpawnPickupInWorldWithClassAndItemEntry(WorldContextObject, ItemEntry, nullptr, Position, Direction, OverrideMaxStackCount, bToss, true, false, SourceType, Source, nullptr, false);
			if (Pickup) {
				Pickups.Add(Pickup);
			}
		}
	}

	return Pickups;
}

TArray<AFortPickup*> UFortKismetLibrary::DropInstancedLootAtLocation(UObject* WorldContextObject, const FName& LootTierGroup, const TArray<AFortPlayerController*>& RelevantPlayers, bool bAllowCombining, const FVector& DropLocation, EFortPickupSourceTypeFlag SourceTypeFlag) {
	TArray<AFortPickup*> Pickups;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) {
		return Pickups;
	}

	const int32 WorldLevel = GetLootWorldLevel(World);
	const FVector Direction = FVector();
	for (AFortPlayerController* RelevantPlayer : RelevantPlayers) {
		if (!RelevantPlayer) {
			continue;
		}

		TArray<FFortItemEntry> LootToDrop;
		if (!PickLootDrops(WorldContextObject, LootToDrop, LootTierGroup, WorldLevel, INDEX_NONE)) {
			continue;
		}

		for (const FFortItemEntry& ItemEntry : LootToDrop) {
			AFortPickup* Pickup = K2_SpawnPickupInWorldWithClassAndItemEntry(WorldContextObject, ItemEntry, nullptr, DropLocation, Direction, 0, true, true, false, SourceTypeFlag, EFortPickupSpawnSource::Unset, RelevantPlayer, true, bAllowCombining);
			if (Pickup) {
				Pickups.Add(Pickup);
			}
		}
	}

	return Pickups;
}

TArray<AFortPickup*> UFortKismetLibrary::DropInstancedLoot(AFortPawn* PawnContext, const FName& LootTierGroup, const TArray<AFortPlayerController*>& RelevantPlayers, bool bAllowCombining) {
	if (!PawnContext) {
		return TArray<AFortPickup*>();
	}

	return DropInstancedLootAtLocation(PawnContext, LootTierGroup, RelevantPlayers, bAllowCombining, PawnContext->K2_GetActorLocation(), EFortPickupSourceTypeFlag::Other);
}

DEFINE_FUNCTION(UFortKismetLibrary::execPickLootDrops)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_TARRAY_REF(FFortItemEntry,Z_Param_Out_OutLootToDrop);
	P_GET_PROPERTY(FNameProperty,Z_Param_TierGroupName);
	P_GET_PROPERTY(FIntProperty,Z_Param_WorldLevel);
	P_GET_PROPERTY(FIntProperty,Z_Param_ForcedLootTier);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UFortKismetLibrary::PickLootDrops(Z_Param_WorldContextObject,Z_Param_Out_OutLootToDrop,Z_Param_TierGroupName,Z_Param_WorldLevel,Z_Param_ForcedLootTier);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execPickLootDropsWithNamedWeights)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_TARRAY_REF(FFortItemEntry,Z_Param_Out_OutLootToDrop);
	P_GET_PROPERTY(FNameProperty,Z_Param_TierGroupName);
	P_GET_PROPERTY(FIntProperty,Z_Param_WorldLevel);
	P_GET_TMAP_REF(FName,float,Z_Param_Out_NamedWeightsMap);
	P_GET_PROPERTY(FIntProperty,Z_Param_ForcedLootTier);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UFortKismetLibrary::PickLootDropsWithNamedWeights(Z_Param_WorldContextObject,Z_Param_Out_OutLootToDrop,Z_Param_TierGroupName,Z_Param_WorldLevel,Z_Param_Out_NamedWeightsMap,Z_Param_ForcedLootTier);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_SpawnPickupInWorldWithLootTier)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_PROPERTY(FNameProperty,Z_Param_LootTierName);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_Position);
	P_GET_PROPERTY(FIntProperty,Z_Param_OverrideMaxStackCount);
	P_GET_UBOOL(Z_Param_bToss);
	P_GET_ENUM(EFortPickupSourceTypeFlag,Z_Param_SourceType);
	P_GET_ENUM(EFortPickupSpawnSource,Z_Param_Source);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray<AFortPickup*>*)Z_Param__Result=UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTier(Z_Param_WorldContextObject,Z_Param_LootTierName,Z_Param_Out_Position,Z_Param_OverrideMaxStackCount,Z_Param_bToss,EFortPickupSourceTypeFlag(Z_Param_SourceType),EFortPickupSpawnSource(Z_Param_Source));
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execDropInstancedLoot)
{
	P_GET_OBJECT(AFortPawn,Z_Param_PawnContext);
	P_GET_PROPERTY_REF(FNameProperty,Z_Param_Out_LootTierGroup);
	P_GET_TARRAY_REF(AFortPlayerController*,Z_Param_Out_RelevantPlayers);
	P_GET_UBOOL(Z_Param_bAllowCombining);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray<AFortPickup*>*)Z_Param__Result=UFortKismetLibrary::DropInstancedLoot(Z_Param_PawnContext,Z_Param_Out_LootTierGroup,Z_Param_Out_RelevantPlayers,Z_Param_bAllowCombining);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execDropInstancedLootAtLocation)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_PROPERTY_REF(FNameProperty,Z_Param_Out_LootTierGroup);
	P_GET_TARRAY_REF(AFortPlayerController*,Z_Param_Out_RelevantPlayers);
	P_GET_UBOOL(Z_Param_bAllowCombining);
	P_GET_STRUCT_REF(FVector,Z_Param_Out_DropLocation);
	P_GET_ENUM(EFortPickupSourceTypeFlag,Z_Param_SourceTypeFlag);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray<AFortPickup*>*)Z_Param__Result=UFortKismetLibrary::DropInstancedLootAtLocation(Z_Param_WorldContextObject,Z_Param_Out_LootTierGroup,Z_Param_Out_RelevantPlayers,Z_Param_bAllowCombining,Z_Param_Out_DropLocation,EFortPickupSourceTypeFlag(Z_Param_SourceTypeFlag));
	P_NATIVE_END;
}

UFortWorldItem* UFortKismetLibrary::GiveItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer, int32 ItemLevel, int32 PickupInstigatorHandle, bool bUseItemPickupAnalyticEvent) {
	UObject* InventoryOwnerObject = InventoryOwner.GetObject();
	AFortPlayerController* PlayerController = InventoryOwnerObject ? InventoryOwnerObject->Cast<AFortPlayerController>() : nullptr;
	if (!PlayerController || !ItemDefinition || NumberToGive <= 0) {
		return nullptr;
	}

	FFortItemEntry ItemEntry(ItemDefinition, NumberToGive, ItemLevel);
	if (PickupInstigatorHandle != INDEX_NONE) {
		ItemEntry.SetStateValue(EFortItemEntryState::PickupInstigatorHandle, PickupInstigatorHandle);
	}
	if (bNotifyPlayer) {
		ItemEntry.SetStateValue(EFortItemEntryState::ShouldShowItemToast, 1);
	}

	return PlayerController->AddInventoryItem(ItemEntry, false);
}

void UFortKismetLibrary::K2_GiveItemToPlayer(AFortPlayerController* PlayerController, const UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer) {
	if (!PlayerController) {
		return;
	}

	TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
	InventoryOwner.SetObject(PlayerController);
	InventoryOwner.SetInterface(&PlayerController->InventoryOwnerInterface);
	GiveItemToInventoryOwner(InventoryOwner, ItemDefinition, NumberToGive, bNotifyPlayer, INDEX_NONE, INDEX_NONE, false);
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_GiveItemToPlayer)
{
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToGive);
	P_GET_UBOOL(Z_Param_bNotifyPlayer);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortKismetLibrary::K2_GiveItemToPlayer(Z_Param_PlayerController,Z_Param_ItemDefinition,Z_Param_NumberToGive,Z_Param_bNotifyPlayer);
	P_NATIVE_END;
}

void UFortKismetLibrary::AddRegenItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const UFortWorldItemDefinition* RegenItemDefinition, int32 NumberToGive, bool bNotifyPlayer, bool bResetRegenCooldown) {
	UObject* InventoryOwnerObject = InventoryOwner.GetObject();
	AFortPlayerController* PlayerController = InventoryOwnerObject ? InventoryOwnerObject->Cast<AFortPlayerController>() : nullptr;
	if (!PlayerController || !RegenItemDefinition || NumberToGive <= 0) {
		return;
	}

	FFortItemEntry ItemEntry(RegenItemDefinition, NumberToGive, INDEX_NONE);
	if (bNotifyPlayer) {
		ItemEntry.SetStateValue(EFortItemEntryState::ShouldShowItemToast, 1);
	}

	PlayerController->AddInventoryItem(ItemEntry, bResetRegenCooldown);
}

DEFINE_FUNCTION(UFortKismetLibrary::execGiveItemToInventoryOwner)
{
	P_GET_TINTERFACE(IFortInventoryOwnerInterface,Z_Param_InventoryOwner);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToGive);
	P_GET_UBOOL(Z_Param_bNotifyPlayer);
	P_GET_PROPERTY(FIntProperty,Z_Param_ItemLevel);
	P_GET_PROPERTY(FIntProperty,Z_Param_PickupInstigatorHandle);
	P_GET_UBOOL(Z_Param_bUseItemPickupAnalyticEvent);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortKismetLibrary::GiveItemToInventoryOwner(Z_Param_InventoryOwner,Z_Param_ItemDefinition,Z_Param_NumberToGive,Z_Param_bNotifyPlayer,Z_Param_ItemLevel,Z_Param_PickupInstigatorHandle,Z_Param_bUseItemPickupAnalyticEvent);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execAddRegenItemToInventoryOwner)
{
	P_GET_TINTERFACE(IFortInventoryOwnerInterface,Z_Param_InventoryOwner);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_RegenItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_NumberToGive);
	P_GET_UBOOL(Z_Param_bNotifyPlayer);
	P_GET_UBOOL(Z_Param_bResetRegenCooldown);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortKismetLibrary::AddRegenItemToInventoryOwner(Z_Param_InventoryOwner,Z_Param_RegenItemDefinition,Z_Param_NumberToGive,Z_Param_bNotifyPlayer,Z_Param_bResetRegenCooldown);
	P_NATIVE_END;
}

int32 UFortKismetLibrary::K2_RemoveItemFromPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval) {
	if (!PlayerController || !PlayerController->WorldInventory || !ItemDefinition) {
		return 0;
	}

	TArray<UFortWorldItem*> Items;
	PlayerController->WorldInventory->FindItemInstancesForDefinition(ItemDefinition, Items, true);

	int32 AmountRemoved = 0;
	for (int32 Index = 0; Index < Items.Num() && (AmountToRemove < 0 || AmountRemoved < AmountToRemove); ++Index) {
		AmountRemoved += PlayerController->RemoveInventoryItem(Items[Index]->GetItemGuid(), AmountToRemove < 0 ? INDEX_NONE : AmountToRemove - AmountRemoved, bForceRemoval);
	}

	return AmountRemoved;
}

int32 UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 AmountToRemove, bool bForceRemoval) {
	if (!PlayerController) {
		return 0;
	}

	return PlayerController->RemoveInventoryItem(ItemGuid, AmountToRemove, bForceRemoval);
}

int32 UFortKismetLibrary::K2_RemoveItemsFromPlayerByIntStateValue(AFortPlayerController* PlayerController, EFortItemEntryState StateType, int32 StateValue, bool bForceRemoval) {
	if (!PlayerController || !PlayerController->WorldInventory) {
		return 0;
	}

	TArray<UFortWorldItem*> Items = PlayerController->WorldInventory->Inventory.ItemInstances;

	int32 ItemsRemoved = 0;
	for (int32 Index = 0; Index < Items.Num(); ++Index) {
		UFortWorldItem* Item = Items[Index];
		int32 ItemStateValue = 0;
		if (Item && Item->ItemEntry.GetStateValue(StateType, ItemStateValue) && ItemStateValue == StateValue) {
			if (PlayerController->RemoveInventoryItem(Item->GetItemGuid(), INDEX_NONE, bForceRemoval) > 0) {
				++ItemsRemoved;
			}
		}
	}

	return ItemsRemoved;
}

int32 UFortKismetLibrary::K2_RemoveItemsFromPlayerByNameStateValue(AFortPlayerController* PlayerController, EFortItemEntryState StateType, FName StateValue, bool bForceRemoval) {
	if (!PlayerController || !PlayerController->WorldInventory) {
		return 0;
	}

	TArray<UFortWorldItem*> Items = PlayerController->WorldInventory->Inventory.ItemInstances;

	int32 ItemsRemoved = 0;
	for (int32 Index = 0; Index < Items.Num(); ++Index) {
		UFortWorldItem* Item = Items[Index];
		FName ItemStateValue;
		if (Item && Item->ItemEntry.GetStateValue(StateType, ItemStateValue) && ItemStateValue == StateValue) {
			if (PlayerController->RemoveInventoryItem(Item->GetItemGuid(), INDEX_NONE, bForceRemoval) > 0) {
				++ItemsRemoved;
			}
		}
	}

	return ItemsRemoved;
}

void UFortKismetLibrary::K2_RemoveItemFromAllPlayers(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove) {
	TArray<AFortPlayerController*> PlayerControllers = GetAllFortPlayerControllers(WorldContextObject, true, false);
	for (int32 Index = 0; Index < PlayerControllers.Num(); ++Index) {
		K2_RemoveItemFromPlayer(PlayerControllers[Index], ItemDefinition, AmountToRemove, false);
	}
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_RemoveItemFromPlayer)
{
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_AmountToRemove);
	P_GET_UBOOL(Z_Param_bForceRemoval);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=UFortKismetLibrary::K2_RemoveItemFromPlayer(Z_Param_PlayerController,Z_Param_ItemDefinition,Z_Param_AmountToRemove,Z_Param_bForceRemoval);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_RemoveItemFromPlayerByGuid)
{
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_STRUCT_REF(FGuid,Z_Param_Out_ItemGuid);
	P_GET_PROPERTY(FIntProperty,Z_Param_AmountToRemove);
	P_GET_UBOOL(Z_Param_bForceRemoval);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid(Z_Param_PlayerController,Z_Param_Out_ItemGuid,Z_Param_AmountToRemove,Z_Param_bForceRemoval);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_RemoveItemsFromPlayerByIntStateValue)
{
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_PROPERTY(FByteProperty,Z_Param_StateType);
	P_GET_PROPERTY(FIntProperty,Z_Param_StateValue);
	P_GET_UBOOL(Z_Param_bForceRemoval);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=UFortKismetLibrary::K2_RemoveItemsFromPlayerByIntStateValue(Z_Param_PlayerController,EFortItemEntryState(Z_Param_StateType),Z_Param_StateValue,Z_Param_bForceRemoval);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_RemoveItemsFromPlayerByNameStateValue)
{
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_PROPERTY(FByteProperty,Z_Param_StateType);
	P_GET_PROPERTY(FNameProperty,Z_Param_StateValue);
	P_GET_UBOOL(Z_Param_bForceRemoval);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=UFortKismetLibrary::K2_RemoveItemsFromPlayerByNameStateValue(Z_Param_PlayerController,EFortItemEntryState(Z_Param_StateType),Z_Param_StateValue,Z_Param_bForceRemoval);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_RemoveItemFromAllPlayers)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_OBJECT(UFortWorldItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_AmountToRemove);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortKismetLibrary::K2_RemoveItemFromAllPlayers(Z_Param_WorldContextObject,Z_Param_ItemDefinition,Z_Param_AmountToRemove);
	P_NATIVE_END;
}

void UFortKismetLibrary::K2_GiveBuildingResource(AFortPlayerController* Controller, const EFortResourceType ResourceType, const int32 ResourceAmount) {
	if (!Controller || ResourceAmount <= 0) {
		return;
	}

	UFortGameData* GameData = UFortAssetManager::Get().GetGameData();
	UFortResourceItemDefinition* ResourceItemDefinition = GameData ? GameData->GetResourceItemDefinition(ResourceType) : nullptr;
	if (!ResourceItemDefinition) {
		return;
	}

	TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
	InventoryOwner.SetObject(Controller);
	InventoryOwner.SetInterface(&Controller->InventoryOwnerInterface);
	GiveItemToInventoryOwner(InventoryOwner, ResourceItemDefinition, ResourceAmount, false, INDEX_NONE, INDEX_NONE, false);
}

bool UFortKismetLibrary::UpgradeAllWeaponsVerticalToRarity(AFortPlayerController* PlayerController, EFortRarity NewRarity, bool bThrottle) {
	if (!PlayerController || !PlayerController->WorldInventory) {
		return false;
	}

	TArray<UFortWorldItem*> Items = PlayerController->WorldInventory->Inventory.ItemInstances;

	bool bUpgradedAllWeapons = true;
	for (int32 Index = 0; Index < Items.Num(); ++Index) {
		UFortWorldItem* Item = Items[Index];
		UFortWeaponItemDefinition* WeaponItemDefinition = Item && Item->ItemEntry.ItemDefinition ? Item->ItemEntry.ItemDefinition->Cast<UFortWeaponItemDefinition>() : nullptr;
		if (!WeaponItemDefinition || WeaponItemDefinition->GetRarity() >= NewRarity) {
			continue;
		}

		UFortWeaponItemDefinition* UpgradedItemDefinition = GetUpgradedWeaponItemVerticalToRarity(WeaponItemDefinition, NewRarity);
		if (!UpgradedItemDefinition || UpgradedItemDefinition == WeaponItemDefinition) {
			bUpgradedAllWeapons = false;
			continue;
		}

		FFortItemEntry UpgradedEntry(UpgradedItemDefinition, Item->ItemEntry.Count, Item->ItemEntry.Level);
		UpgradedEntry.LoadedAmmo = Item->ItemEntry.LoadedAmmo;

		PlayerController->RemoveInventoryItem(Item->GetItemGuid(), INDEX_NONE, true);
		if (!PlayerController->AddInventoryItem(UpgradedEntry, false)) {
			bUpgradedAllWeapons = false;
		}
	}

	return bUpgradedAllWeapons;
}

DEFINE_FUNCTION(UFortKismetLibrary::execK2_GiveBuildingResource)
{
	P_GET_OBJECT(AFortPlayerController,Z_Param_Controller);
	P_GET_PROPERTY(FByteProperty,Z_Param_ResourceType);
	P_GET_PROPERTY(FIntProperty,Z_Param_ResourceAmount);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortKismetLibrary::K2_GiveBuildingResource(Z_Param_Controller,EFortResourceType(Z_Param_ResourceType),Z_Param_ResourceAmount);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UFortKismetLibrary::execUpgradeAllWeaponsVerticalToRarity)
{
	P_GET_OBJECT(AFortPlayerController,Z_Param_PlayerController);
	P_GET_ENUM(EFortRarity,Z_Param_NewRarity);
	P_GET_UBOOL(Z_Param_bThrottle);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UFortKismetLibrary::UpgradeAllWeaponsVerticalToRarity(Z_Param_PlayerController,EFortRarity(Z_Param_NewRarity),Z_Param_bThrottle);
	P_NATIVE_END;
}

void UFortKismetLibrary::Init() {
	Memory::HookDetour(ImageBase + 0x5098E58, execIncrementAnalyticMatchCount, nullptr);
	Memory::HookDetour(ImageBase + 0x508ADCC, execChangeTeam, nullptr);
	Memory::HookDetour(ImageBase + 0x509B410, execK2_SpawnPickupInWorld, nullptr);
	Memory::HookDetour(ImageBase + 0x509B870, execK2_SpawnPickupInWorldWithClass, nullptr);
	Memory::HookDetour(ImageBase + 0x509BD24, execK2_SpawnPickupInWorldWithClassAndLevel, nullptr);
	Memory::HookDetour(ImageBase + 0x509C224, execK2_SpawnPickupInWorldWithLevel, nullptr);
	Memory::HookDetour(ImageBase + 0x50A0FD8, execSpawnInstancedPickupInWorld, nullptr);
	Memory::HookDetour(ImageBase + 0x50A12C8, execSpawnItemVariantPickupInWorld, nullptr);
	Memory::HookDetour(ImageBase + 0x509E684, execPickLootDrops, nullptr);
	Memory::HookDetour(ImageBase + 0x509E858, execPickLootDropsWithNamedWeights, nullptr);
	Memory::HookDetour(ImageBase + 0x509C6D8, execK2_SpawnPickupInWorldWithLootTier, nullptr);
	Memory::HookDetour(ImageBase + 0x508DB48, execDropInstancedLoot, nullptr);
	Memory::HookDetour(ImageBase + 0x508DCE0, execDropInstancedLootAtLocation, nullptr);
	Memory::HookDetour(ImageBase + 0x509A70C, execK2_GiveItemToPlayer, nullptr);
	Memory::HookDetour(ImageBase + 0x5098800, execGiveItemToInventoryOwner, nullptr);
	Memory::HookDetour(ImageBase + 0x5087E10, execAddRegenItemToInventoryOwner, nullptr);
	Memory::HookDetour(ImageBase + 0x509AC40, execK2_RemoveItemFromPlayer, nullptr);
	Memory::HookDetour(ImageBase + 0x509AED8, execK2_RemoveItemFromPlayerByGuid, nullptr);
	Memory::HookDetour(ImageBase + 0x509B04C, execK2_RemoveItemsFromPlayerByIntStateValue, nullptr);
	Memory::HookDetour(ImageBase + 0x509B1BC, execK2_RemoveItemsFromPlayerByNameStateValue, nullptr);
	Memory::HookDetour(ImageBase + 0x509ADB4, execK2_RemoveItemFromAllPlayers, nullptr);
	Memory::HookDetour(ImageBase + 0x509A5E8, execK2_GiveBuildingResource, nullptr);
	Memory::HookDetour(ImageBase + 0x50A2B18, execUpgradeAllWeaponsVerticalToRarity, nullptr);
}

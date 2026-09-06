#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"
#include "FortniteGame/Source/FortniteGame/Public/FortPickup.h"

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

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldWithClassAndItemEntry(UObject* WorldContextObject, const FFortItemEntry& ItemEntry, TSubclassOf<AFortPickup> PickupClass, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner) {
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
		Pickup->TossPickup(bToss ? Position + Direction : Position, nullptr, OverrideMaxStackCount, bToss, true, SourceType, Source);
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

void UFortKismetLibrary::Init() {
	Memory::HookDetour(ImageBase + 0x5098E58, execIncrementAnalyticMatchCount, nullptr);
	Memory::HookDetour(ImageBase + 0x508ADCC, execChangeTeam, nullptr);
	Memory::HookDetour(ImageBase + 0x509B410, execK2_SpawnPickupInWorld, nullptr);
	Memory::HookDetour(ImageBase + 0x509B870, execK2_SpawnPickupInWorldWithClass, nullptr);
	Memory::HookDetour(ImageBase + 0x509BD24, execK2_SpawnPickupInWorldWithClassAndLevel, nullptr);
	Memory::HookDetour(ImageBase + 0x509C224, execK2_SpawnPickupInWorldWithLevel, nullptr);
	Memory::HookDetour(ImageBase + 0x50A0FD8, execSpawnInstancedPickupInWorld, nullptr);
	Memory::HookDetour(ImageBase + 0x50A12C8, execSpawnItemVariantPickupInWorld, nullptr);
}

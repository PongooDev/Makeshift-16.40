#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/FortAssets.h"
#include "FortniteGame/Source/FortniteGame/Public/FortGlobals.h"
#include "FortniteGame/Source/FortniteGame/Public/Items/FortPickup.h"

static void GetOwnedGameplayTags(const UObject* Object, FGameplayTagContainer& OutTags) {
	const IGameplayTagAssetInterface* TagInterface = Object ? static_cast<const IGameplayTagAssetInterface*>(const_cast<UObject*>(Object)->GetInterfaceAddress(IGameplayTagAssetInterface::StaticClass())) : nullptr;
	if (TagInterface) {
		TagInterface->GetOwnedGameplayTags(OutTags);
	}
}

bool ABuildingContainer::SpawnLoot(AFortPlayerPawn* PlayerPawn, const EFortPickupSourceTypeFlag InSourceTypeFlag, const EFortPickupSpawnSource InSpawnSource, const bool bLootFromDestruction) {
	UE_LOG(LogFortBuilding, VeryVerbose, TEXT("ABuildingContainer::SpawnLoot() called on %s (%hs)..."), *GetPathName(), Name.ToString().c_str());

	if (Role < ENetRole::ROLE_Authority) {
		return false;
	}

	UWorld* World = GetWorld();
	if (!World) {
		return false;
	}

	if (!bLootFromDestruction) {
		FVector BounceNormal = PlayerPawn ? PlayerPawn->GetActorLocation() - GetActorLocation() : FVector(0.f, 0.f, 0.f);
		if (!BounceNormal.IsZero()) {
			BounceNormal.Normalize();
		}

		SearchBounceData.BounceNormal = BounceNormal;
		SearchBounceData.SearchingPawn = PlayerPawn;
		++SearchBounceData.SearchAnimationCount;
		BounceContainer();
	}

	if (IsAlreadySearched() || !IsAllowedToSpawnLoot()) {
		return false;
	}

	AFortPlayerController* PlayerController = PlayerPawn && PlayerPawn->Controller ? PlayerPawn->Controller->Cast<AFortPlayerController>() : nullptr;

	FName TierGroupName = SearchLootTierGroup;
	if (PlayerPawn && LootTierGroupTagOverrideData.Num() > 0) {
		FGameplayTagContainer PlayerTags;
		GetOwnedGameplayTags(PlayerPawn->AbilitySystemComponent, PlayerTags);
		for (const FLootTierGroupTagOverride& TagOverride : LootTierGroupTagOverrideData) {
			if (TagOverride.IsEnabled.GetValueAtLevel(0.f) > 0.f && TagOverride.PlayerTagQuery.Matches(PlayerTags)) {
				TierGroupName = TagOverride.OverrideLootTierGroup;
				break;
			}
		}
	}

	const AFortGameState* GameState = World->GameState ? World->GameState->Cast<AFortGameState>() : nullptr;
	TArray<FFortItemEntry> LootToDrop;
	UFortKismetLibrary::PickLootDrops(this, LootToDrop, TierGroupName, GameState ? GameState->WorldLevel : 0, ReplicatedLootTier);
	ModifyLootDrops(LootToDrop, PlayerController);

	EFortPickupSpawnSource SpawnSource = InSpawnSource;
	if (SpawnSource == EFortPickupSpawnSource::Unset) {
		FGameplayTagContainer OwnedTags;
		GetOwnedGameplayTags(this, OwnedTags);
		if (OwnedTags.HasTag(UFortGlobals::GameplayTags().BuildingContainer_TreasureChest)) {
			SpawnSource = EFortPickupSpawnSource::Chest;
		} else if (OwnedTags.HasTag(UFortGlobals::GameplayTags().BuildingContainer_AmmoBox)) {
			SpawnSource = EFortPickupSpawnSource::AmmoBox;
		}
	}

	const FVector SpawnLocation = UKismetMathLibrary::TransformLocation(GetTransform(), LootSpawnLocation_Athena);
	const UGameDataBR* GameDataBR = UGameDataBR::Get();
	const UFortWeaponPickupSpawnAmmoData* WeaponPickupSpawnAmmoData = GameDataBR ? FFortAssets::GetAsset(GameDataBR->FortWeaponPickupSpawnAmmoData) : nullptr;

	bool bSpawnedLoot = false;
	EFortRarity HighestSpawnedRarity = EFortRarity::Common;
	for (int32 Index = 0; Index < LootToDrop.Num(); ++Index) {
		const FFortItemEntry& ItemEntry = LootToDrop[Index];
		if (!ItemEntry.ItemDefinition) {
			continue;
		}

		FFortPickupCreationData CreationData(World, ItemEntry, SpawnLocation, FRotator::ZeroRotator, nullptr, nullptr, this);
		CreationData.SourceTypeFlags = InSourceTypeFlag;
		CreationData.SpawnSource = SpawnSource;
		AFortPickup* Pickup = AFortPickup::CreateFromData(CreationData);
		if (!Pickup) {
			continue;
		}

		bSpawnedLoot = true;
		if (ItemEntry.ItemDefinition->Rarity > HighestSpawnedRarity) {
			HighestSpawnedRarity = ItemEntry.ItemDefinition->Rarity;
		}

		OnPickupSpawn(Pickup, PlayerController);
		UFortKismetLibrary::TossPickupFromContainer(this, this, Pickup, LootToDrop.Num(), Index, LootTossConeHalfAngle_Athena, LootTossDirection_Athena, LootTossSpeed_Athena, bForceHidePickupMinimapIndicator);

		const UFortWeaponItemDefinition* WeaponItemDefinition = ItemEntry.ItemDefinition->Cast<UFortWeaponItemDefinition>();
		if (WeaponItemDefinition && WeaponPickupSpawnAmmoData) {
			WeaponPickupSpawnAmmoData->GenerateAmmoPickupForWeaponItemDefinitionFromContainer(*World, *WeaponItemDefinition, *this, InSourceTypeFlag, SpawnSource);
		}
	}

	if (bSpawnedLoot) {
		HighestRarity = HighestSpawnedRarity;
	}

	if (!bDestroyed) {
		if (!bAlwaysMaintainLoot) {
			SetContainerSearched(PlayerPawn);
		}

		if (ShouldDestroyContainerOnSearch()) {
			Die(UFortGlobals::GameplayTags().EffectInstantDeathSilent, nullptr, nullptr, nullptr);
		}
	}

	return bSpawnedLoot;
}

bool ABuildingContainer::SpawnLootHook(ABuildingContainer* This, AFortPlayerPawn* PlayerPawn, const EFortPickupSourceTypeFlag InSourceTypeFlag, const EFortPickupSpawnSource InSpawnSource, const bool bLootFromDestruction) {
	return This->SpawnLoot(PlayerPawn, InSourceTypeFlag, InSpawnSource, bLootFromDestruction);
}

void ABuildingContainer::Init() {
	void* const SpawnLootStub = reinterpret_cast<void*>(ImageBase + 0x4757468);
	UClass* ContainerClass = ABuildingContainer::StaticClass();
	for (int32 Index = 0; Index < UObject::GObjects->Num(); ++Index) {
		UObject* Object = UObject::GObjects->GetByIndex(Index);
		if (Object && Object->IsDefaultObject() && Object->IsA(ContainerClass) && Object->VTable[473] == SpawnLootStub) {
			Memory::HookVTable(Object, 473, SpawnLootHook);
		}
	}
}

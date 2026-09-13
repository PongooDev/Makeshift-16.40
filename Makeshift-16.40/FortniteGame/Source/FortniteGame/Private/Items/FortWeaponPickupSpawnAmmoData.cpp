#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/Items/FortPickup.h"

static void GetOwnedGameplayTags(const UObject* Object, FGameplayTagContainer& OutTags) {
	const IGameplayTagAssetInterface* TagInterface = Object ? static_cast<const IGameplayTagAssetInterface*>(const_cast<UObject*>(Object)->GetInterfaceAddress(IGameplayTagAssetInterface::StaticClass())) : nullptr;
	if (TagInterface) {
		TagInterface->GetOwnedGameplayTags(OutTags);
	}
}

AFortPickup* UFortWeaponPickupSpawnAmmoData::GenerateAmmoPickupForWeaponItemDefinitionFromContainer(UWorld& InWorld, const UFortWeaponItemDefinition& WeaponItemDefinition, ABuildingContainer& Container, const EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource) const {
	UFortWorldItemDefinition* AmmoWorldItemDefinition = WeaponItemDefinition.GetAmmoWorldItemDefinition(false);
	const UFortAmmoItemDefinition* AmmoItemDefinition = AmmoWorldItemDefinition ? AmmoWorldItemDefinition->Cast<UFortAmmoItemDefinition>() : nullptr;
	if (!AmmoItemDefinition) {
		return nullptr;
	}

	FGameplayTagContainer WeaponTags;
	GetOwnedGameplayTags(&WeaponItemDefinition, WeaponTags);
	FGameplayTagContainer SourceTags;
	GetOwnedGameplayTags(&Container, SourceTags);

	const FVector InitialPosition = UKismetMathLibrary::TransformLocation(Container.GetTransform(), Container.LootSpawnLocation_Athena);

	FFortItemEntry NewPickupData(AmmoItemDefinition, 1, 0);
	UpdateAmmoItemEntryStackCount(InWorld, *AmmoItemDefinition, WeaponTags, SourceTags, NewPickupData);
	if (NewPickupData.Count <= 0) {
		return nullptr;
	}

	FFortPickupCreationData CreationData(&InWorld, NewPickupData, InitialPosition, FRotator::ZeroRotator, nullptr, nullptr, &Container);
	CreationData.SourceTypeFlags = SourceTypeFlag;
	CreationData.SpawnSource = SpawnSource;
	AFortPickup* Pickup = AFortPickup::CreateFromData(CreationData);
	if (!Pickup) {
		return nullptr;
	}

	const int32 NumSteps = 10;
	UFortKismetLibrary::TossPickupFromContainer(&InWorld, &Container, Pickup, NumSteps, FMath::RandRange(0, NumSteps), Container.LootTossConeHalfAngle_Athena, Container.LootTossDirection_Athena, Container.LootTossSpeed_Athena, Container.bForceHidePickupMinimapIndicator);
	return Pickup;
}

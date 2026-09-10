#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/Items/FortPickup.h"

FFortPickupCreationData::FFortPickupCreationData(UWorld* InWorld, const FFortItemEntry& InPickupDataItemEntry, const FVector& InPosition, const FRotator& InRotation, AFortPlayerController* InOptionalPCOwner, TSubclassOf<AFortPickup> InOverrideClass, AActor* InOwnerContainer)
	: Position(InPosition)
	, Rotation(InRotation) {
	void (*Fn)(FFortPickupCreationData*, UWorld*, const FFortItemEntry*, const FVector*, const FRotator*, AFortPlayerController*, UClass*, AActor*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x413BB64);
	Fn(this, InWorld, &InPickupDataItemEntry, &InPosition, &InRotation, InOptionalPCOwner, InOverrideClass, InOwnerContainer);
}

AFortPickup* AFortPickup::CreateFromData(const FFortPickupCreationData& CreationData) {
	AFortPickup* (*Fn)(const FFortPickupCreationData*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x4A51ED0);
	return Fn(&CreationData);
}

void AFortPickup::GivePickupTo(IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup) {
	if (Role == ENetRole::ROLE_Authority) {
		if (InventoryOwner) {
			GivePickupItemsToInventoryOwner(InventoryOwner);
			OnPickupGivenTo(InventoryOwner);
			UpdateSpecialActorStat(ESpecialActorStatType::PickupNumTaken);
			if (DestoryAfterPickup) {
				Destroy();
			}
		}
	}
}

void AFortPickup::GivePickupItemsToInventoryOwner(IFortInventoryOwnerInterface* InventoryOwner) {
	UObject* OwnerObject = InventoryOwner ? InventoryOwner->_getUObject() : nullptr;
	AFortPlayerController* PlayerController = OwnerObject ? OwnerObject->Cast<AFortPlayerController>() : nullptr;
	if (!PlayerController || bIsVisualOnlyPickup) {
		return;
	}

	PlayerController->AddInventoryItem(PrimaryPickupItemEntry, false);
	for (int32 EntryIndex = 0; EntryIndex < MultiItemPickupEntries.Num(); ++EntryIndex) {
		PlayerController->AddInventoryItem(MultiItemPickupEntries[EntryIndex], false);
	}
}

void AFortPickup::GivePickupToHook(AFortPickup* This, IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup) {
	This->GivePickupTo(InventoryOwner, DestoryAfterPickup);
}

void AFortPickup::GivePickupItemsToInventoryOwnerHook(AFortPickup* This, IFortInventoryOwnerInterface* InventoryOwner) {
	This->GivePickupItemsToInventoryOwner(InventoryOwner);
}

void AFortPickup::Init() {
	Memory::SwapVTableEntryInAllSubClasses<AFortPickup>(204, GivePickupToHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPickup>(220, GivePickupItemsToInventoryOwnerHook);
}

#include "pch.h"

void FFortItemEntry::SetParentInventory(AFortInventory* InParentInventory, bool InIsReplicatedCopy) {
	ParentInventory = InParentInventory;
	bIsReplicatedCopy = InIsReplicatedCopy;
}

void AFortInventory::InitializeExistingItem(UFortWorldItem* ExistingItem) {
	if (!ExistingItem) {
		return;
	}

	ExistingItem->OwnerInventory = this;
	ExistingItem->ItemEntry.SetParentInventory(this, false);
	ExistingItem->bIsTemporaryItem = false;
	ExistingItem->bNeedsPersistentUpdate = true;
	ExistingItem->GetItemGuid();

	InitializeExistingItemInternal(ExistingItem);
}

UFortWorldItem* AFortInventory::AddItem(const FFortItemEntry& ItemEntry) {
	const UFortItemDefinition* ItemDefinition = ItemEntry.GetItemDefinition();
	if (!ItemDefinition) {
		return nullptr;
	}

	UFortItem* Item = ItemDefinition->CreateTemporaryItemInstance(ItemEntry);
	UFortWorldItem* WorldItem = Item ? Item->Cast<UFortWorldItem>() : nullptr;
	if (!WorldItem) {
		return nullptr;
	}

	InitializeExistingItem(WorldItem);

	return WorldItem;
}

static void RemoveReplicatedEntryAtSwap(TArray<FFortItemEntry>& ReplicatedEntries, int32 Index) {
	void (*Fn)(TArray<FFortItemEntry>*, int32, int32, bool) = decltype(Fn)(ImageBase + 0x1DDC030);
	Fn(&ReplicatedEntries, Index, 1, true);
}

void AFortInventory::OnRemoveItemStack(UFortWorldItem* ItemStackToRemove, const FGuid& ItemGuid) {
	if (!ItemStackToRemove) {
		return;
	}

	FFortItemEntry RemovedEntry;
	for (int32 Index = 0; Index < Inventory.ReplicatedEntries.Num(); ++Index) {
		if (Inventory.ReplicatedEntries[Index].ItemGuid == ItemGuid) {
			RemovedEntry = Inventory.ReplicatedEntries[Index];
			RemoveReplicatedEntryAtSwap(Inventory.ReplicatedEntries, Index);
			break;
		}
	}

	int32 PickupInstigatorHandle = INDEX_NONE;
	if (RemovedEntry.GetStateValue(EFortItemEntryState::PickupInstigatorHandle, PickupInstigatorHandle)) {
		APlayerController* PlayerController = Owner ? Owner->Cast<APlayerController>() : nullptr;
		if (PlayerController) {
			UFortPickupInstigatorRegistryComponent* PickupRegistry = UFortPickupInstigatorRegistryComponent::GetPickupRegistryForContext(PlayerController->PlayerState);
			if (PickupRegistry) {
				PickupRegistry->NotifyItemRemoved(PickupInstigatorHandle, PlayerController, RemovedEntry);
			}
		}
	}

	UpdateItemInstances();
	HandleInventoryItemRemoved();
}

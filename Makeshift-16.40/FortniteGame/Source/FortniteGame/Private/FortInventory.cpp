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

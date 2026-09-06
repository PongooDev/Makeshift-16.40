#include "pch.h"

void FFortItemEntry::SetParentInventory(AFortInventory* InParentInventory, bool InIsReplicatedCopy) {
	ParentInventory = InParentInventory;
	bIsReplicatedCopy = InIsReplicatedCopy;
}

UFortWorldItem* AFortInventory::AddItem(const FFortItemEntry& ItemEntry) {
	const int32 EntryIndex = Inventory.ReplicatedEntries.Add(ItemEntry);
	FFortItemEntry& ReplicatedEntry = Inventory.ReplicatedEntries[EntryIndex];
	ReplicatedEntry.SetParentInventory(this, true);
	const FGuid ItemGuid = ReplicatedEntry.GetItemGuid();
	ReplicatedEntry.SetToDirty();

	UpdateItemInstances();

	return GetItem(ItemGuid);
}

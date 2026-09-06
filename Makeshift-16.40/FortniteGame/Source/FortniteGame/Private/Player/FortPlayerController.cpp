#include "pch.h"

UFortWorldItem* AFortPlayerController::AddInventoryItem(const FFortItemEntry& ItemEntry, bool bResetRegenCooldown) {
	const UFortItemDefinition* ItemDefinition = ItemEntry.GetItemDefinition();
	if (!WorldInventory || !ItemDefinition || ItemEntry.Count <= 0) {
		return nullptr;
	}

	IFortInventoryOwnerInterface* InventoryOwner = &InventoryOwnerInterface;
	AFortPawn* FortPawn = InventoryOwner->GetFortPawn();
	UAbilitySystemComponent* AbilitySystemComponent = FortPawn ? FortPawn->AbilitySystemComponent : nullptr;

	const int32 MaxStackSize = ItemDefinition->GetMaxStackSize(AbilitySystemComponent);
	const int32 StackSize = MaxStackSize > 0 ? MaxStackSize : ItemEntry.Count;
	const int32 MaxNumStacks = ItemDefinition->GetMaxNumStacks();

	int32 ShowItemToast = 0;
	ItemEntry.GetStateValue(EFortItemEntryState::ShouldShowItemToast, ShowItemToast);

	const int32 OverflowCount = FMath::Clamp(InventoryOwner->CountInventoryOverflowFromAddingItem(ItemEntry, false, false), 0, ItemEntry.Count);
	int32 CountRemaining = ItemEntry.Count - OverflowCount;
	UFortWorldItem* LastItem = nullptr;

	TArray<UFortWorldItem*> ExistingItems;
	WorldInventory->FindItemInstancesForDefinition(ItemDefinition, ExistingItems, false);
	int32 NumStacks = ExistingItems.Num();

	for (int32 Index = 0; Index < ExistingItems.Num() && CountRemaining > 0; ++Index) {
		UFortWorldItem* ExistingItem = ExistingItems[Index];
		const int32 CurrentCount = ExistingItem->ItemEntry.Count;
		const int32 CountToAdd = FMath::Min(StackSize - CurrentCount, CountRemaining);
		if (CountToAdd > 0 && ExistingItem->SetNumInStack(CurrentCount + CountToAdd, bResetRegenCooldown)) {
			if (ShowItemToast) {
				ExistingItem->ItemEntry.SetStateValue(EFortItemEntryState::ShouldShowItemToast, ShowItemToast);
			}
			CountRemaining -= CountToAdd;
			LastItem = ExistingItem;
		}
	}

	bool bFirstNewStack = true;
	while (CountRemaining > 0 && (MaxNumStacks < 0 || NumStacks < MaxNumStacks)) {
		FFortItemEntry NewEntry(ItemEntry);
		NewEntry.SetParentInventory(nullptr, false);
		if (!bFirstNewStack) {
			NewEntry.SetItemGuid(FGuid());
		}
		NewEntry.SetCount(FMath::Min(StackSize, CountRemaining));

		UFortWorldItem* NewItem = WorldInventory->AddItem(NewEntry);
		if (!NewItem) {
			break;
		}
		ItemDefinition->OnItemInstanceAdded(InventoryOwner, NewItem, false);

		CountRemaining -= NewEntry.Count;
		++NumStacks;
		bFirstNewStack = false;
		LastItem = NewItem;
	}

	const int32 CountToDrop = OverflowCount + CountRemaining;
	if (CountToDrop > 0 && FortPawn) {
		FFortItemEntry PickupEntry(ItemEntry);
		PickupEntry.SetParentInventory(nullptr, false);
		PickupEntry.SetItemGuid(FGuid());
		PickupEntry.SetCount(CountToDrop);
		UFortKismetLibrary::K2_SpawnPickupInWorldWithClassAndItemEntry(this, PickupEntry, nullptr, FortPawn->K2_GetActorLocation(), FVector(), 0, true, true, false, EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource::Unset, this, false, true);
	}

	if (LastItem) {
		WorldInventory->HandleInventoryLocalUpdate();
	}

	return LastItem;
}

int32 AFortPlayerController::RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoval) {
	if (!WorldInventory) {
		return 0;
	}

	UFortWorldItem* Item = WorldInventory->InventoryInterface.GetItem(ItemGuid);
	if (!Item) {
		return 0;
	}

	const int32 CurrentCount = Item->ItemEntry.Count;
	const int32 CountToRemove = Count < 0 ? CurrentCount : FMath::Min(Count, CurrentCount);
	if (CountToRemove <= 0) {
		return 0;
	}

	UFortItemDefinition* ItemDefinition = Item->ItemEntry.ItemDefinition;
	UFortWorldItemDefinition* WorldItemDefinition = ItemDefinition ? ItemDefinition->Cast<UFortWorldItemDefinition>() : nullptr;
	const bool bKeepEmptyStack = !bForceRemoval && WorldItemDefinition && WorldItemDefinition->bPersistInInventoryWhenFinalStackEmpty;

	if (CountToRemove < CurrentCount || bKeepEmptyStack) {
		Item->SetNumInStack(CurrentCount - CountToRemove, false);
	} else {
		if (ItemDefinition) {
			ItemDefinition->OnItemInstanceRemoved(&InventoryOwnerInterface, Item);
		}
		WorldInventory->OnRemoveItemStack(Item, ItemGuid);
	}

	WorldInventory->HandleInventoryLocalUpdate();

	return CountToRemove;
}

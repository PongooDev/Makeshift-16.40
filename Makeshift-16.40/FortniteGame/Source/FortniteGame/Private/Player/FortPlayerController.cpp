#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/FortPickup.h"

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

bool AFortPlayerController::RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars, bool bForceRemoval, bool bForcePersistWhenEmpty) {
	if (!WorldInventory) {
		return false;
	}

	UFortWorldItem* Item = WorldInventory->InventoryInterface.GetItem(ItemGuid);
	if (!Item) {
		return false;
	}

	const int32 CurrentCount = Item->ItemEntry.Count;
	const int32 CountToRemove = Count < 0 ? CurrentCount : FMath::Min(Count, CurrentCount);
	if (CountToRemove <= 0) {
		return false;
	}

	UFortItemDefinition* ItemDefinition = Item->ItemEntry.ItemDefinition;
	UFortWorldItemDefinition* WorldItemDefinition = ItemDefinition ? ItemDefinition->Cast<UFortWorldItemDefinition>() : nullptr;
	const bool bKeepEmptyStack = !bForceRemoval && (bForcePersistWhenEmpty || (WorldItemDefinition && WorldItemDefinition->bPersistInInventoryWhenFinalStackEmpty));

	if (bForceRemoveFromQuickBars) {
		AddDelayedQuickBarAction(EFortDelayedQuickBarAction::Remove, Item, EFortQuickBars::Max_None, INDEX_NONE, false);
	}

	if (CountToRemove < CurrentCount || bKeepEmptyStack) {
		Item->SetNumInStack(CurrentCount - CountToRemove, false);
	} else {
		if (ItemDefinition) {
			ItemDefinition->OnItemInstanceRemoved(&InventoryOwnerInterface, Item);
		}
		WorldInventory->OnRemoveItemStack(Item, ItemGuid);
		WorldInventory->SetRequiresUpdate();
	}

	WorldInventory->HandleInventoryLocalUpdate();

	return true;
}

int32 AFortPlayerController::RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoval) {
	UFortWorldItem* Item = WorldInventory ? WorldInventory->InventoryInterface.GetItem(ItemGuid) : nullptr;
	if (!Item) {
		return 0;
	}

	const int32 CountToRemove = Count < 0 ? Item->ItemEntry.Count : FMath::Min(Count, Item->ItemEntry.Count);
	return RemoveInventoryItem(ItemGuid, CountToRemove, false, bForceRemoval, false) ? CountToRemove : 0;
}

bool AFortPlayerController::RemoveInventoryItemHook(IFortInventoryOwnerInterface* This, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars, bool bForceRemoval, bool bForcePersistWhenEmpty) {
	AFortPlayerController* PlayerController = reinterpret_cast<AFortPlayerController*>(reinterpret_cast<uint8*>(This) - offsetof(AFortPlayerController, InventoryOwnerInterface));
	return PlayerController->RemoveInventoryItem(ItemGuid, Count, bForceRemoveFromQuickBars, bForceRemoval, bForcePersistWhenEmpty);
}

void AFortPlayerController::ServerExecuteInventoryItem_Implementation(const FGuid& ItemGuid) {
	UFortItem* Item = BP_GetInventoryItemWithGuid(ItemGuid);
	const UFortItemDefinition* ItemDefinition = Item ? Item->GetItemDefinition() : nullptr;
	if (ItemDefinition && ItemDefinition->IsA(UFortWorldItemDefinition::StaticClass())) {
		static_cast<const UFortWorldItemDefinition*>(ItemDefinition)->ServerExecute(Item, this);
	}
}

void AFortPlayerController::ServerExecuteInventoryItemHook(AFortPlayerController* This, const FGuid& ItemGuid) {
	This->ServerExecuteInventoryItem_Implementation(ItemGuid);
}

void AFortPlayerController::ServerAttemptInventoryDrop_Implementation(const FGuid& ItemGuid, int32 Count, bool bTrash) {
	if (!WorldInventory) {
		return;
	}

	UFortWorldItem* Item = WorldInventory->InventoryInterface.GetItem(ItemGuid);
	if (!Item || !Item->CanBeDropped()) {
		return;
	}

	const UFortItemDefinition* ItemDefinition = Item->GetItemDefinition();
	if (!ItemDefinition || !ItemDefinition->IsA(UFortWorldItemDefinition::StaticClass())) {
		return;
	}

	const UFortWorldItemDefinition* WorldItemDefinition = static_cast<const UFortWorldItemDefinition*>(ItemDefinition);
	const int32 NumInStack = Item->ItemEntry.Count;
	if (Count <= 0 || Count > NumInStack) {
		return;
	}

	AFortPickup* Pickup = nullptr;
	if (!bTrash && MyFortPawn && WorldItemDefinition->DropBehavior != EWorldItemDropBehavior::DestroyOnDrop) {
		FFortItemEntry PickupEntry(Item->ItemEntry);
		PickupEntry.SetCount(Count);
		if (Count < NumInStack) {
			PickupEntry.SetItemGuid(FGuid::NewGuid());
		}

		FFortPickupCreationData CreationData(GetWorld(), PickupEntry, MyFortPawn->GetActorLocation(), FRotator::ZeroRotator, nullptr, nullptr, nullptr);
		Pickup = AFortPickup::CreateFromData(CreationData);
	}

	if (MyFortPawn && Count >= NumInStack) {
		MyFortPawn->UnequipCurrentWeaponById(ItemGuid, true);
	}

	const bool bForceRemoval = bTrash || WorldItemDefinition->DropBehavior == EWorldItemDropBehavior::DropAsPickupDestroyOnEmpty;
	RemoveInventoryItem(ItemGuid, Count, bForceRemoval);

	if (Pickup) {
		const FVector FinalLocation = MyFortPawn->GetActorLocation() + MyFortPawn->GetActorForwardVector() * 512.f;
		Pickup->TossPickup(FinalLocation, MyFortPawn, 0, true, true, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset);
	}
}

void AFortPlayerController::ServerAttemptInventoryDropHook(AFortPlayerController* This, const FGuid& ItemGuid, int32 Count, bool bTrash) {
	This->ServerAttemptInventoryDrop_Implementation(ItemGuid, Count, bTrash);
}

void AFortPlayerController::ServerCombineInventoryItems_Implementation(const FGuid& TargetItemGuid, const FGuid& SourceItemGuid) {
	if (!WorldInventory || TargetItemGuid == SourceItemGuid) {
		return;
	}

	UFortWorldItem* TargetItem = WorldInventory->InventoryInterface.GetItem(TargetItemGuid);
	UFortWorldItem* SourceItem = WorldInventory->InventoryInterface.GetItem(SourceItemGuid);
	if (!TargetItem || !SourceItem) {
		return;
	}

	const UFortItemDefinition* ItemDefinition = TargetItem->GetItemDefinition();
	if (!ItemDefinition || ItemDefinition != SourceItem->GetItemDefinition()) {
		return;
	}

	AFortPawn* FortPawn = InventoryOwnerInterface.GetFortPawn();
	UAbilitySystemComponent* AbilitySystemComponent = FortPawn ? FortPawn->AbilitySystemComponent : nullptr;
	const int32 MaxStackSize = ItemDefinition->GetMaxStackSize(AbilitySystemComponent);
	const int32 TargetCount = TargetItem->ItemEntry.Count;
	const int32 CountToMove = FMath::Min(MaxStackSize - TargetCount, SourceItem->ItemEntry.Count);
	if (CountToMove <= 0) {
		return;
	}

	if (!TargetItem->SetNumInStack(TargetCount + CountToMove, false)) {
		return;
	}

	RemoveInventoryItem(SourceItemGuid, CountToMove, true);
}

void AFortPlayerController::ServerCombineInventoryItemsHook(AFortPlayerController* This, const FGuid& TargetItemGuid, const FGuid& SourceItemGuid) {
	This->ServerCombineInventoryItems_Implementation(TargetItemGuid, SourceItemGuid);
}

void AFortPlayerController::AddDelayedQuickBarAction(EFortDelayedQuickBarAction Action, const UFortItem* Item, EFortQuickBars QuickBarType, int32 QuickBarSlot, bool bForceExecution) {
	const UFortItemDefinition* ItemDefinition = Item ? Item->GetItemDefinition() : nullptr;
	if (Action == EFortDelayedQuickBarAction::Invalid || !ItemDefinition) {
		return;
	}

	FDelayedQuickBarAction DelayedAction;
	DelayedAction.Action = Action;
	DelayedAction.ItemGuid = Item->GetItemGuid();
	DelayedAction.ItemDefinitionAssetId = UKismetSystemLibrary::GetPrimaryAssetIdFromObject(const_cast<UFortItemDefinition*>(ItemDefinition));
	DelayedAction.QuickBarType = QuickBarType;
	DelayedAction.QuickBarSlot = QuickBarSlot;
	DelayedAction.bForceExecution = bForceExecution;
	AddDelayedQuickBarAction(DelayedAction);
}

void AFortPlayerController::AddDelayedQuickBarAction(FDelayedQuickBarAction DelayedAction) {
	if (DelayedAction.Action == EFortDelayedQuickBarAction::Invalid || !DelayedAction.ItemGuid.IsValid() || !DelayedAction.ItemDefinitionAssetId.IsValid()) {
		return;
	}

	DelayedAction.ActionId = ++DelayedQuickBarActions.CurrentItemId;
	const int32 NewIndex = DelayedQuickBarActions.Items.Add(DelayedAction);
	DelayedQuickBarActions.MarkItemDirty(DelayedQuickBarActions.Items[NewIndex]);
}

void AFortPlayerController::ServerAcknowledgeDelayedQuickBarAction_Implementation(const TArray<uint32>& ProcessedActionIds) {
	bool bRemovedAction = false;
	for (int32 Index = DelayedQuickBarActions.Items.Num() - 1; Index >= 0; --Index) {
		if (ProcessedActionIds.Contains(DelayedQuickBarActions.Items[Index].ActionId)) {
			DelayedQuickBarActions.Items.RemoveAt(Index);
			bRemovedAction = true;
		}
	}

	if (bRemovedAction) {
		DelayedQuickBarActions.MarkArrayDirty();
	}
}

void AFortPlayerController::ServerAcknowledgeDelayedQuickBarActionHook(AFortPlayerController* This, const TArray<uint32>& ProcessedActionIds) {
	This->ServerAcknowledgeDelayedQuickBarAction_Implementation(ProcessedActionIds);
}

void AFortPlayerController::ServerOnMaterialSelection_Implementation(EFortResourceType NewResourceType, EFortResourceLevel NewResourceLevel) {
	CurrentResourceType = NewResourceType;
	CurrentResourceLevel = NewResourceLevel;
}

void AFortPlayerController::ServerOnMaterialSelectionHook(AFortPlayerController* This, EFortResourceType NewResourceType, EFortResourceLevel NewResourceLevel) {
	This->ServerOnMaterialSelection_Implementation(NewResourceType, NewResourceLevel);
}

void AFortPlayerController::Init() {
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(45, RemoveInventoryItemHook, offsetof(AFortPlayerController, InventoryOwnerInterface));
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(532, ServerExecuteInventoryItemHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(548, ServerAttemptInventoryDropHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(550, ServerCombineInventoryItemsHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(552, ServerAcknowledgeDelayedQuickBarActionHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(557, ServerOnMaterialSelectionHook);
}

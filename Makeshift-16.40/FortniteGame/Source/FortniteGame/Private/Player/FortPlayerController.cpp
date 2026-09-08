#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/World.h"
#include "FortniteGame/Source/FortniteGame/Public/FortGlobals.h"
#include "FortniteGame/Source/FortniteGame/Public/Items/FortPickup.h"
#include "FortniteGame/Source/FortniteGame/Public/FortAssets.h"

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

void AFortPlayerController::ServerCreateBuildingActor_Implementation(const FCreateBuildingActorData& CreateBuildingData) {
	UWorld* World = GetWorld();
	AFortGameStateZone* GameState = World && World->GameState ? World->GameState->Cast<AFortGameStateZone>() : nullptr;
	if (!GameState) {
		return;
	}

	FBuildingClassData BuildingClassData = CreateBuildingData.BuildingClassData;
	const int32 BuildingClassHandle = static_cast<int32>(CreateBuildingData.BuildingClassHandle);
	if (BuildingClassHandle >= 0 && BuildingClassHandle < GameState->AllPlayerBuildableClasses.Num()) {
		BuildingClassData.BuildingClass = GameState->AllPlayerBuildableClasses[BuildingClassHandle];
	}

	ServerCreateBuildingActorInternal(BuildingClassData, CreateBuildingData.BuildLoc, CreateBuildingData.BuildRot, CreateBuildingData.bMirrored, CreateBuildingData.SyncKey, false, false, false);
}

ABuildingSMActor* AFortPlayerController::ServerCreateBuildingActorInternal(FBuildingClassData BuildingClassData, FVector_NetQuantize10 BuildLoc, FRotator BuildRot, bool bMirrored, float SyncKey, bool bIgnoreInteractBuildCheck, bool bIgnoreExtraPieceCost, bool bIgnoreBuildingValidityCheck) {
	UWorld* World = GetWorld();
	UClass* BuildingClass = BuildingClassData.BuildingClass.Get();
	if (!World || !BuildingClass || !BuildingClass->IsSubclassOf(ABuildingSMActor::StaticClass())) {
		return nullptr;
	}

	const FFortGlobalGameplayTags& GameplayTags = UFortGlobals::GameplayTags();
	if (!bIgnoreInteractBuildCheck && !CanPerformNativeAction(GameplayTags.ActionPlayerInteractBuild)) {
		return nullptr;
	}

	if (!bIgnoreBuildingValidityCheck && IsBuildingRestricted(BuildingClassData, BuildLoc, BuildRot)) {
		return nullptr;
	}

	TArray<ABuildingActor*> ExistingBuildings;
	EFortBuildPreviewMarkerOptionalAdjustment BuildPreviewMarkerOptionalAdjustment = EFortBuildPreviewMarkerOptionalAdjustment::None;
	if (CanPlaceBuildableClassInStructuralGrid(BuildingClass, BuildLoc, BuildRot, bMirrored, ExistingBuildings, BuildPreviewMarkerOptionalAdjustment) != EFortStructuralGridQueryResults::CanAdd) {
		return nullptr;
	}

	if (!CanAffordToPlaceBuildableClass(BuildingClassData)) {
		return nullptr;
	}

	if (ShouldDestroyBuildingsOnPlacement()) {
		for (int32 Index = 0; Index < ExistingBuildings.Num(); ++Index) {
			ABuildingActor* ExistingBuilding = ExistingBuildings[Index];
			if (ExistingBuilding && ExistingBuilding->bDestroyOnPlayerBuildingPlacement) {
				ExistingBuilding->Die(GameplayTags.EffectInstantDeathStructuralSupport, nullptr, nullptr, nullptr);
			}
		}
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bDeferConstruction = true;
	ABuildingSMActor* BuildingActor = ABuildingActor::SpawnBuilding(World, BuildingClass, BuildLoc, BuildRot, SpawnInfo);
	if (!BuildingActor) {
		return nullptr;
	}

	BuildingActor->SetCurrentBuildingLevel(BuildingClassData.UpgradeLevel);
	BuildingActor->SetMirrored(bMirrored);
	BuildingActor->InitializeBuildingActor(EFortBuildingInitializationReason::Spawned, GetWorldPlayerId(), nullptr, nullptr, false);
	BuildingActor->PostInitializeSpawnedBuildingActor(EFortBuildingInitializationReason::Spawned);
	BuildingActor->FinishSpawning(FTransform(BuildRot, BuildLoc));

	PayBuildableClassPlacementCost(BuildingClassData);

	if (BuildPreviewMarkerOptionalAdjustment != EFortBuildPreviewMarkerOptionalAdjustment::None) {
		BuildExtraPieceForSupport(World, BuildingClass, BuildLoc, BuildRot, bMirrored, BuildingClassData.UpgradeLevel);
		if (!bIgnoreExtraPieceCost) {
			PayBuildableClassPlacementCost(BuildingClassData);
		}
	}

	if (MyFortPawn) {
		UFortAIFunctionLibrary::MakeNoiseEventAtLocation(MyFortPawn, 1000.f, BuildingActor->GetActorLocation(), FName(L"Building"));
		MyFortPawn->OnBlueprintPlace();
		MyFortPawn->ResetSpawnImmunityTime();
	}

	return BuildingActor;
}

void AFortPlayerController::ServerCreateBuildingActorHook(AFortPlayerController* This, const FCreateBuildingActorData& CreateBuildingData) {
	This->ServerCreateBuildingActor_Implementation(CreateBuildingData);
}

void AFortPlayerController::ServerBeginEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit) {
	AFortPlayerStateZone* PlayerStateZone = PlayerState ? PlayerState->Cast<AFortPlayerStateZone>() : nullptr;
	if (!BuildingActorToEdit || !PlayerStateZone || BuildingActorToEdit->IsBeingPlayerEdited() || !IsInEditingRange(BuildingActorToEdit) || !BuildingActorToEdit->CanBePlayerEdited(this)) {
		ClientFailedToBeginEditingBuildingActor(BuildingActorToEdit);
		return;
	}

	BuildingActorToEdit->SetEditingPlayer(PlayerStateZone);
}

void AFortPlayerController::ServerBeginEditingBuildingActorHook(AFortPlayerController* This, ABuildingSMActor* BuildingActorToEdit) {
	This->ServerBeginEditingBuildingActor_Implementation(BuildingActorToEdit);
}

void AFortPlayerController::ServerEditBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored) {
	AFortPlayerStateZone* PlayerStateZone = PlayerState ? PlayerState->Cast<AFortPlayerStateZone>() : nullptr;
	if (!BuildingActorToEdit || !PlayerStateZone || BuildingActorToEdit->GetEditingPlayer() != PlayerStateZone) {
		return;
	}

	UWorld* World = GetWorld();
	AFortGameStateZone* GameState = World && World->GameState ? World->GameState->Cast<AFortGameStateZone>() : nullptr;
	UClass* EditClass = NewBuildingClass.Get();
	bool bCanEditIntoClass = false;
	if (GameState && EditClass && EditClass->IsSubclassOf(ABuildingSMActor::StaticClass())) {
		const ABuildingSMActor* DefaultBuildingActor = static_cast<const ABuildingSMActor*>(BuildingActorToEdit->Class->DefaultObject);
		FPlayerBuildableClassFilter Filter{};
		Filter.ResourceType = BuildingActorToEdit->ResourceType;
		Filter.BuildingType = BuildingActorToEdit->BuildingType;
		Filter.Level = BuildingActorToEdit->bUpgradeUsesSameClass && DefaultBuildingActor ? DefaultBuildingActor->GetCurrentBuildingLevel() : BuildingActorToEdit->GetCurrentBuildingLevel();
		Filter.EditModeMetadata = nullptr;

		TArray<TSubclassOf<ABuildingSMActor>> PossibleEditClasses;
		GameState->GetPlayerBuildableClasses(PossibleEditClasses, Filter);
		for (int32 Index = 0; Index < PossibleEditClasses.Num(); ++Index) {
			if (PossibleEditClasses[Index].Get() == EditClass) {
				bCanEditIntoClass = true;
				break;
			}
		}
	}

	if (bCanEditIntoClass) {
		DoEditBuildingActorAnalytics(BuildingActorToEdit);
		if (BuildingActorToEdit->ReplaceBuildingActor(EBuildingReplacementType::BRT_Edited, EditClass, BuildingActorToEdit->GetCurrentBuildingLevel(), RotationIterations, bMirrored, this)) {
			return;
		}
	}

	BuildingActorToEdit->SetEditingPlayer(nullptr);
}

void AFortPlayerController::ServerEditBuildingActorHook(AFortPlayerController* This, ABuildingSMActor* BuildingActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored) {
	This->ServerEditBuildingActor_Implementation(BuildingActorToEdit, NewBuildingClass, RotationIterations, bMirrored);
}

void AFortPlayerController::ServerEndEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToStopEditing) {
	AFortPlayerStateZone* PlayerStateZone = PlayerState ? PlayerState->Cast<AFortPlayerStateZone>() : nullptr;
	if (!BuildingActorToStopEditing || !PlayerStateZone || BuildingActorToStopEditing->GetEditingPlayer() != PlayerStateZone) {
		return;
	}

	BuildingActorToStopEditing->SetEditingPlayer(nullptr);
}

void AFortPlayerController::ServerEndEditingBuildingActorHook(AFortPlayerController* This, ABuildingSMActor* BuildingActorToStopEditing) {
	This->ServerEndEditingBuildingActor_Implementation(BuildingActorToStopEditing);
}

void AFortPlayerController::ServerRepairBuildingActor_Implementation(ABuildingSMActor* BuildingActorToRepair) {
	if (!BuildingActorToRepair || !BuildingActorToRepair->NeedsRepair() || !CanAffordToRepair(BuildingActorToRepair)) {
		return;
	}

	const int32 ResourcesSpent = PayBuildingRepairCost(BuildingActorToRepair);
	BuildingActorToRepair->RepairBuilding(this, ResourcesSpent);
}

void AFortPlayerController::ServerRepairBuildingActorHook(AFortPlayerController* This, ABuildingSMActor* BuildingActorToRepair) {
	This->ServerRepairBuildingActor_Implementation(BuildingActorToRepair);
}

void AFortPlayerController::ServerUpgradeBuildingActor_Implementation(ABuildingActor* BuildingActorToUpgrade, int32 NewUpgradeLevel) {
	if (!BuildingActorToUpgrade) {
		return;
	}

	BuildingActorToUpgrade->OnServerAttemptBuildingUpgrade(this, NewUpgradeLevel);
}

void AFortPlayerController::ServerUpgradeBuildingActorHook(AFortPlayerController* This, ABuildingActor* BuildingActorToUpgrade, int32 NewUpgradeLevel) {
	This->ServerUpgradeBuildingActor_Implementation(BuildingActorToUpgrade, NewUpgradeLevel);
}

void AFortPlayerController::DropItemsOnPawnDestruction(EPawnDestructionReason DestructionReason, const FGameplayTagContainer& ContextualTags, AFortPawn* DestructionPawn, bool& bOutDroppedBackpack) {
	bOutDroppedBackpack = false;
	if (Role != ENetRole::ROLE_Authority || !ShouldDropItemsBasedOnTags(DestructionReason, ContextualTags)) {
		return;
	}

	AFortPlayerPawn* DeadPawn = DestructionPawn ? DestructionPawn->Cast<AFortPlayerPawn>() : nullptr;
	if (DeadPawn) {
		bOutDroppedBackpack = HandleBackpackDrop(DeadPawn);
	}
}

void AFortPlayerController::DropItemsOnPawnDestructionHook(AFortPlayerController* This, EPawnDestructionReason DestructionReason, const FGameplayTagContainer& ContextualTags, AFortPawn* DestructionPawn, bool& bOutDroppedBackpack) {
	This->DropItemsOnPawnDestruction(DestructionReason, ContextualTags, DestructionPawn, bOutDroppedBackpack);
}

void AFortPlayerController::DropItemsAsPickupsAsync(TArray<UFortWorldItem*>& ItemsToDropViaPickup, AFortPawn* DestructionPawn) {
	if (ItemsToDropViaPickup.Num() <= 0) {
		return;
	}

	QueuedItemsToDrop.DestructionPawn = DestructionPawn;
	QueuedItemsToDrop.TotalNumItemsToDrop = ItemsToDropViaPickup.Num();
	QueuedItemsToDrop.ItemsToDrop = ItemsToDropViaPickup;
	DropItemsAsPickups(QueuedItemsToDrop.ItemsToDrop, QueuedItemsToDrop.DestructionPawn, this, QueuedItemsToDrop.TotalNumItemsToDrop, QueuedItemsToDrop.TotalNumItemsToDrop);
	QueuedItemsToDrop.ItemsToDrop.Empty();
	QueuedItemsToDrop.TotalNumItemsToDrop = 0;
	QueuedItemsToDrop.DestructionPawn = nullptr;
}

void AFortPlayerController::DropItemsAsPickupsAsyncHook(AFortPlayerController* This, TArray<UFortWorldItem*>& ItemsToDropViaPickup, AFortPawn* DestructionPawn) {
	This->DropItemsAsPickupsAsync(ItemsToDropViaPickup, DestructionPawn);
}

void AFortPlayerController::DropItemsAsPickups(TArray<UFortWorldItem*>& ItemsToDropViaPickup, AFortPawn* DestructionPawn, AFortPlayerController* InFortPlayerController, int32 NumToDrop, int32 OriginalTotalNumItems) {
	UWorld* World = GetWorld();
	const int32 NumToDropNow = FMath::Min(NumToDrop, ItemsToDropViaPickup.Num());
	if (!World || !DestructionPawn || !InFortPlayerController || NumToDropNow <= 0) {
		return;
	}

	const FVector DropLocation = DestructionPawn->GetActorLocation();
	for (int32 Index = 0; Index < NumToDropNow; ++Index) {
		UFortWorldItem* Item = ItemsToDropViaPickup[Index];
		if (!Item) {
			continue;
		}

		FFortPickupCreationData CreationData(World, Item->ItemEntry, DropLocation, FRotator::ZeroRotator, nullptr, nullptr, nullptr);
		AFortPickup* Pickup = AFortPickup::CreateFromData(CreationData);
		InFortPlayerController->RemoveInventoryItem(Item->ItemEntry.ItemGuid, Item->ItemEntry.Count, false, true, false);
		if (Pickup) {
			Pickup->TossPickup(DropLocation, DestructionPawn, 0, true, true, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);
		}
	}

	ItemsToDropViaPickup.RemoveAt(0, NumToDropNow);
}

void AFortPlayerController::DropItemsAsPickupsHook(IFortInventoryOwnerInterface* This, TArray<UFortWorldItem*>& ItemsToDropViaPickup, AFortPawn* DestructionPawn, AFortPlayerController* InFortPlayerController, int32 NumToDrop, int32 OriginalTotalNumItems) {
	AFortPlayerController* PlayerController = reinterpret_cast<AFortPlayerController*>(reinterpret_cast<uint8*>(This) - offsetof(AFortPlayerController, InventoryOwnerInterface));
	PlayerController->DropItemsAsPickups(ItemsToDropViaPickup, DestructionPawn, InFortPlayerController, NumToDrop, OriginalTotalNumItems);
}

static FFortPickupTossOverrideData DefaultPickupTossOverrideData() {
	FFortPickupTossOverrideData TossOverrideData{};
	TossOverrideData.bIsValid = true;
	TossOverrideData.MinTossDist = 300.f;
	TossOverrideData.MaxTossDist = 600.f;
	TossOverrideData.SpawnDirectionConeHalfAngle = 45.f;
	return TossOverrideData;
}

static void DropInventoryItemFromPawn(AFortPlayerController* PlayerController, UFortWorldItem* Item, bool bSpawnPickups, const FFortPickupTossOverrideData& TossOverrideData) {
	AFortPlayerPawn* FortPawn = PlayerController->MyFortPawn;
	const FGuid ItemGuid = Item->ItemEntry.ItemGuid;
	const int32 Count = Item->ItemEntry.Count;

	AFortPickup* Pickup = nullptr;
	if (bSpawnPickups && FortPawn) {
		FFortPickupCreationData CreationData(PlayerController->GetWorld(), Item->ItemEntry, FortPawn->GetActorLocation(), FRotator::ZeroRotator, nullptr, nullptr, nullptr);
		Pickup = AFortPickup::CreateFromData(CreationData);
	}

	if (FortPawn) {
		FortPawn->UnequipCurrentWeaponById(ItemGuid, true);
	}

	PlayerController->RemoveInventoryItem(ItemGuid, Count, false, true, false);

	if (Pickup) {
		const FVector Forward = FortPawn->GetActorForwardVector();
		const float Angle = FMath::FRandRange(-TossOverrideData.SpawnDirectionConeHalfAngle, TossOverrideData.SpawnDirectionConeHalfAngle) * (PI / 180.f);
		const float CosAngle = FMath::Cos(Angle);
		const float SinAngle = FMath::Sin(Angle);
		FVector Direction;
		Direction.X = Forward.X * CosAngle - Forward.Y * SinAngle;
		Direction.Y = Forward.X * SinAngle + Forward.Y * CosAngle;
		Direction.Z = 0.f;
		const FVector FinalLocation = FortPawn->GetActorLocation() + Direction * FMath::FRandRange(TossOverrideData.MinTossDist, TossOverrideData.MaxTossDist);
		Pickup->TossPickup(FinalLocation, FortPawn, 0, true, true, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset);
	}
}

static void DropItemsOfDefinition(AFortPlayerController* PlayerController, const UFortItemDefinition* DropItemDef, bool bSpawnPickups, const FFortPickupTossOverrideData& TossOverrideData) {
	if (PlayerController->Role != ENetRole::ROLE_Authority || !PlayerController->WorldInventory || !DropItemDef) {
		return;
	}

	TArray<UFortWorldItem*> ItemsToDrop;
	PlayerController->WorldInventory->FindItemInstancesForDefinition(DropItemDef, ItemsToDrop, false);
	for (int32 Index = 0; Index < ItemsToDrop.Num(); ++Index) {
		if (ItemsToDrop[Index]) {
			DropInventoryItemFromPawn(PlayerController, ItemsToDrop[Index], bSpawnPickups, TossOverrideData);
		}
	}
}

void AFortPlayerController::ServerDropAllItems_Implementation(const UFortItemDefinition* IgnoreItemDef) {
	if (!IgnoreItemDef) {
		return;
	}

	const bool bIgnoreBuildingMaterials = IgnoreItemDef->HasMatchingGameplayTag(UFortGlobals::GameplayTags().WeaponKeepsMaterialsOnDropAll);
	DropAllItems(IgnoreItemDef, nullptr, bIgnoreBuildingMaterials, true);
}

void AFortPlayerController::ServerDropAllItemsHook(AFortPlayerController* This, const UFortItemDefinition* IgnoreItemDef) {
	This->ServerDropAllItems_Implementation(IgnoreItemDef);
}

void AFortPlayerController::DropAllItems(const UFortItemDefinition* IgnoreItemDef, const UFortItemDefinition* AdditionalIgnoreItemDef, bool bIgnoreBuildingMaterials, bool bSpawnPickups) {
	if (Role != ENetRole::ROLE_Authority || !WorldInventory) {
		return;
	}

	TArray<UFortWorldItem*> ItemsToDrop;
	const TArray<UFortWorldItem*>& ItemInstances = WorldInventory->Inventory.ItemInstances;
	for (int32 Index = 0; Index < ItemInstances.Num(); ++Index) {
		UFortWorldItem* Item = ItemInstances[Index];
		const UFortItemDefinition* ItemDefinition = Item ? Item->GetItemDefinition() : nullptr;
		if (!ItemDefinition || ItemDefinition == IgnoreItemDef || ItemDefinition == AdditionalIgnoreItemDef || !Item->CanBeDropped()) {
			continue;
		}

		if (bIgnoreBuildingMaterials && ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass())) {
			continue;
		}

		ItemsToDrop.Add(Item);
	}

	const FFortPickupTossOverrideData TossOverrideData = DefaultPickupTossOverrideData();
	for (int32 Index = 0; Index < ItemsToDrop.Num(); ++Index) {
		DropInventoryItemFromPawn(this, ItemsToDrop[Index], bSpawnPickups, TossOverrideData);
	}
}

void AFortPlayerController::DropSpecificItem(const UFortItemDefinition* DropItemDef) {
	DropItemsOfDefinition(this, DropItemDef, true, DefaultPickupTossOverrideData());
}

void AFortPlayerController::TossSpecificItem(const UFortItemDefinition* DropItemDef, const FFortPickupTossOverrideData& TossOverrideData) {
	DropItemsOfDefinition(this, DropItemDef, true, TossOverrideData.bIsValid ? TossOverrideData : DefaultPickupTossOverrideData());
}

DEFINE_FUNCTION(AFortPlayerController::execDropAllItems)
{
	P_GET_OBJECT(UFortItemDefinition,Z_Param_IgnoreItemDef);
	P_GET_OBJECT(UFortItemDefinition,Z_Param_AdditionalIgnoreItemDef);
	P_GET_UBOOL(Z_Param_bIgnoreBuildingMaterials);
	P_GET_UBOOL(Z_Param_bSpawnPickups);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(AFortPlayerController)->DropAllItems(Z_Param_IgnoreItemDef,Z_Param_AdditionalIgnoreItemDef,Z_Param_bIgnoreBuildingMaterials,Z_Param_bSpawnPickups);
	P_NATIVE_END;
}

DEFINE_FUNCTION(AFortPlayerController::execDropSpecificItem)
{
	P_GET_OBJECT(UFortItemDefinition,Z_Param_DropItemDef);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(AFortPlayerController)->DropSpecificItem(Z_Param_DropItemDef);
	P_NATIVE_END;
}

DEFINE_FUNCTION(AFortPlayerController::execTossSpecificItem)
{
	P_GET_OBJECT(UFortItemDefinition,Z_Param_DropItemDef);
	P_GET_STRUCT_REF(FFortPickupTossOverrideData,Z_Param_Out_TossOverrideData);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(AFortPlayerController)->TossSpecificItem(Z_Param_DropItemDef,Z_Param_Out_TossOverrideData);
	P_NATIVE_END;
}

void AFortPlayerController::SetInventoryStateValue(FGuid ItemGuid, const FFortItemEntryStateValue& StateValue) {
	if (Role != ENetRole::ROLE_Authority) {
		ServerSetInventoryStateValue(ItemGuid, StateValue);
		return;
	}

	UFortWorldItem* Item = WorldInventory ? WorldInventory->InventoryInterface.GetItem(ItemGuid) : nullptr;
	if (!Item) {
		return;
	}

	Item->ItemEntry.SetStateValue(StateValue);
	WorldInventory->HandleInventoryLocalUpdate();
}

void AFortPlayerController::RemoveInventoryStateValue(FGuid ItemGuid, EFortItemEntryState StateValueType) {
	if (Role != ENetRole::ROLE_Authority) {
		ServerRemoveInventoryStateValue(ItemGuid, StateValueType);
		return;
	}

	UFortWorldItem* Item = WorldInventory ? WorldInventory->InventoryInterface.GetItem(ItemGuid) : nullptr;
	if (!Item || !Item->ItemEntry.RemoveStateValue(StateValueType)) {
		return;
	}

	WorldInventory->HandleInventoryLocalUpdate();
}

void AFortPlayerController::ServerSetInventoryStateValue_Implementation(FGuid ItemGuid, FFortItemEntryStateValue StateValue) {
	SetInventoryStateValue(ItemGuid, StateValue);
}

void AFortPlayerController::ServerSetInventoryStateValueHook(AFortPlayerController* This, const FGuid& ItemGuid, const FFortItemEntryStateValue& StateValue) {
	This->ServerSetInventoryStateValue_Implementation(ItemGuid, StateValue);
}

void AFortPlayerController::ServerRemoveInventoryStateValue_Implementation(FGuid ItemGuid, EFortItemEntryState StateValueType) {
	RemoveInventoryStateValue(ItemGuid, StateValueType);
}

void AFortPlayerController::ServerRemoveInventoryStateValueHook(AFortPlayerController* This, const FGuid& ItemGuid, EFortItemEntryState StateValueType) {
	This->ServerRemoveInventoryStateValue_Implementation(ItemGuid, StateValueType);
}

bool AFortPlayerController::ModDurability(FGuid ItemGuid, float Durability, bool bForceSet) {
	if ((!bForceSet && Durability == 0.f) || Role != ENetRole::ROLE_Authority) {
		return false;
	}

	UFortWorldItem* Item = WorldInventory ? WorldInventory->InventoryInterface.GetItem(ItemGuid) : nullptr;
	const UFortItemDefinition* ItemDefinition = Item ? Item->GetItemDefinition() : nullptr;
	if (!ItemDefinition || !ItemDefinition->IsA(UFortWorldItemDefinition::StaticClass()) || !Item->HasDurability()) {
		return false;
	}

	const UFortWorldItemDefinition* WorldItemDefinition = static_cast<const UFortWorldItemDefinition*>(ItemDefinition);
	const float MaxDurability = WorldItemDefinition->GetMaxDurability(Item->ItemEntry.Level);
	float NewDurability = bForceSet ? Durability : Item->ItemEntry.Durability + Durability;
	if (MaxDurability > 0.f) {
		NewDurability = FMath::Min(NewDurability, MaxDurability);
	}
	NewDurability = FMath::Max(NewDurability, 0.f);
	Item->ItemEntry.SetDurability(NewDurability);

	int32 DurabilityInitialized = 0;
	if (!Item->ItemEntry.GetStateValue(EFortItemEntryState::DurabilityInitialized, DurabilityInitialized) || !DurabilityInitialized) {
		Item->ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
	}

	if (NewDurability <= 0.f && HandleItemZeroDurability(ItemGuid)) {
		RemoveInventoryItem(ItemGuid, Item->ItemEntry.Count, false, true, false);
		return true;
	}

	WorldInventory->HandleInventoryLocalUpdate();
	return true;
}

bool AFortPlayerController::ModDurabilityHook(IFortInventoryOwnerInterface* This, const FGuid& ItemGuid, float Durability, bool bForceSet) {
	AFortPlayerController* PlayerController = reinterpret_cast<AFortPlayerController*>(reinterpret_cast<uint8*>(This) - offsetof(AFortPlayerController, InventoryOwnerInterface));
	return PlayerController->ModDurability(ItemGuid, Durability, bForceSet);
}

bool AFortPlayerController::ForceEquipValidWeapon() {
	if (Role != ENetRole::ROLE_Authority || !MyFortPawn || !WorldInventory) {
		return false;
	}

	const TArray<UFortWorldItem*>& ItemInstances = WorldInventory->Inventory.ItemInstances;
	for (int32 Index = 0; Index < ItemInstances.Num(); ++Index) {
		UFortWorldItem* Item = ItemInstances[Index];
		const UFortItemDefinition* ItemDefinition = Item ? Item->GetItemDefinition() : nullptr;
		if (!ItemDefinition || !ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()) || ItemDefinition->GetItemType() != EFortItemType::WeaponHarvest) {
			continue;
		}

		if (MyFortPawn->CurrentWeapon && MyFortPawn->CurrentWeapon->WeaponData == ItemDefinition) {
			return true;
		}

		return static_cast<const UFortWorldItemDefinition*>(ItemDefinition)->ServerExecute(Item, this);
	}

	return false;
}

bool AFortPlayerController::ForceEquipValidWeaponHook(AFortPlayerController* This) {
	return This->ForceEquipValidWeapon();
}

static bool ActivateEmoteAbility(AFortPlayerPawn* FortPawn, const UFortMontageItemDefinitionBase* EmoteAsset) {
	UAbilitySystemComponent* AbilitySystemComponent = FortPawn ? FortPawn->AbilitySystemComponent : nullptr;
	if (!AbilitySystemComponent || !EmoteAsset || EmoteAsset->IsCosmeticDenied(nullptr)) {
		return false;
	}

	const UGameDataCosmetics& GameDataCosmetics = UGameDataCosmetics::Get();
	UClass* EmoteAbilityClass = nullptr;
	if (EmoteAsset->IsA(UAthenaToyItemDefinition::StaticClass())) {
		EmoteAbilityClass = FFortAssets::GetSubclassOf(static_cast<const UAthenaToyItemDefinition*>(EmoteAsset)->ToySpawnAbility, true);
	} else if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass())) {
		EmoteAbilityClass = FFortAssets::GetSubclassOf(GameDataCosmetics.SprayGameplayAbility, true);
	} else if (EmoteAsset->IsA(UAthenaDanceItemDefinition::StaticClass())) {
		EmoteAbilityClass = FFortAssets::GetSubclassOf(static_cast<const UAthenaDanceItemDefinition*>(EmoteAsset)->CustomDanceAbility, true);
	}

	if (!EmoteAbilityClass) {
		EmoteAbilityClass = FFortAssets::GetSubclassOf(GameDataCosmetics.EmoteGameplayAbility, true);
	}

	if (!EmoteAbilityClass || !EmoteAbilityClass->IsSubclassOf(UFortGameplayAbility::StaticClass())) {
		return false;
	}

	UGameplayAbility* EmoteAbility = EmoteAbilityClass->DefaultObject ? EmoteAbilityClass->DefaultObject->Cast<UGameplayAbility>() : nullptr;
	if (!EmoteAbility) {
		return false;
	}

	FGameplayAbilitySpec Spec(EmoteAbility, 1, INDEX_NONE, const_cast<UFortMontageItemDefinitionBase*>(EmoteAsset));
	AbilitySystemComponent->GiveAbilityAndActivateOnce(Spec);

	const UFortItemDefinition* PreviousEmote = FortPawn->LastReplicatedEmoteExecuted;
	FortPawn->LastReplicatedEmoteExecuted = const_cast<UFortMontageItemDefinitionBase*>(EmoteAsset);
	FortPawn->OnRep_LastReplicatedEmoteExecuted(PreviousEmote);
	return true;
}

void AFortPlayerController::ServerPlayEmoteItem_Implementation(const UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber) {
	if (!MyFortPawn || !CanProbablyPlayEmote(EmoteAsset)) {
		return;
	}

	if (UWorld* World = GetWorld()) {
		MyFortPawn->EmoteStartTime = World->GetTimeSeconds();
	}
	MyFortPawn->EmoteRandomNum = EmoteRandomNumber;

	ActivateEmoteAbility(MyFortPawn, EmoteAsset);
}

void AFortPlayerController::ServerPlayEmoteItemHook(AFortPlayerController* This, const UFortMontageItemDefinitionBase* EmoteAsset, float EmoteRandomNumber) {
	This->ServerPlayEmoteItem_Implementation(EmoteAsset, EmoteRandomNumber);
}

void AFortPlayerController::ServerSpotActor_Implementation(AActor* NewlySpottedActor) {
	AFortTeamInfo* TeamInfo = UFortKismetLibrary::GetActorTeamInfo(this);
	if (!TeamInfo) {
		return;
	}

	TeamInfo->SpotActorForTeam(NewlySpottedActor, this);
}

void AFortPlayerController::ServerSpotActorHook(AFortPlayerController* This, AActor* NewlySpottedActor) {
	This->ServerSpotActor_Implementation(NewlySpottedActor);
}

void AFortPlayerController::Init() {
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(45, RemoveInventoryItemHook, offsetof(AFortPlayerController, InventoryOwnerInterface));
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(532, ServerExecuteInventoryItemHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(548, ServerAttemptInventoryDropHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(550, ServerCombineInventoryItemsHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(552, ServerAcknowledgeDelayedQuickBarActionHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(557, ServerOnMaterialSelectionHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(567, ServerCreateBuildingActorHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(574, ServerBeginEditingBuildingActorHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(569, ServerEditBuildingActorHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(572, ServerEndEditingBuildingActorHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(563, ServerRepairBuildingActorHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(565, ServerUpgradeBuildingActorHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(17, DropItemsAsPickupsHook, offsetof(AFortPlayerController, InventoryOwnerInterface));
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(891, DropItemsOnPawnDestructionHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(893, DropItemsAsPickupsAsyncHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(451, ServerDropAllItemsHook);
	Memory::HookDetour(ImageBase + 0x51315A8, execDropAllItems, nullptr);
	Memory::HookDetour(ImageBase + 0x3BEFE48, execDropSpecificItem, nullptr);
	Memory::HookDetour(ImageBase + 0x513D47C, execTossSpecificItem, nullptr);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(524, ServerSetInventoryStateValueHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(522, ServerRemoveInventoryStateValueHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(18, ModDurabilityHook, offsetof(AFortPlayerController, InventoryOwnerInterface));
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(738, ForceEquipValidWeaponHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(464, ServerPlayEmoteItemHook);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerController>(595, ServerSpotActorHook);
}

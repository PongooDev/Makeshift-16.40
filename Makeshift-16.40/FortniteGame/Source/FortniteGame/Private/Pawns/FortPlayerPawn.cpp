#include "pch.h"

void AFortPlayerPawn::ServerHandlePickupInfo_Implementation(AFortPickup* Pickup, FFortPickupRequestInfo Params) {
	if (!Pickup || Pickup->bPickedUp) {
		return;
	}

	AFortPlayerController* PlayerController = Controller ? Controller->Cast<AFortPlayerController>() : nullptr;
	if (!PlayerController) {
		return;
	}

	if (Params.bIsAutoPickup && !Pickup->IsAllowedToAutoPickUpItemOnServer(PlayerController)) {
		return;
	}

	if (Pickup->ShouldBlockPickup(this) || !Pickup->CheckForRePickup(this)) {
		return;
	}

	if (Params.bIsVisualOnlyPickup) {
		Pickup->MarkVisualOnlyPickup();
	}

	FGuid SwapWithItem = Params.SwapWithItem;
	if (Params.bTrySwapWithWeapon && !SwapWithItem.IsValid() && CurrentWeapon) {
		IFortInventoryOwnerInterface* InventoryOwner = GetInventoryOwnerInterface();
		if (InventoryOwner && InventoryOwner->CountInventoryOverflowFromAddingItem(Pickup->PrimaryPickupItemEntry, false, true) > 0) {
			SwapWithItem = CurrentWeapon->ItemEntryGuid;
		}
	}

	if ((Params.bUseRequestedSwap || Params.bTrySwapWithWeapon) && SwapWithItem.IsValid() && PlayerController->WorldInventory) {
		UFortWorldItem* SwapItem = PlayerController->WorldInventory->InventoryInterface.GetItem(SwapWithItem);
		if (SwapItem && SwapItem->CanBeDropped()) {
			PlayerController->ServerAttemptInventoryDrop_Implementation(SwapWithItem, SwapItem->ItemEntry.Count, false);
		}
	}

	const float FlyTime = Params.FlyTime / PickupSpeedMultiplier;
	Pickup->SetPickupTarget(this, FlyTime, Params.Direction, Params.bPlayPickupSound);
}

void AFortPlayerPawn::ServerHandlePickupInfoHook(AFortPlayerPawn* This, AFortPickup* Pickup, FFortPickupRequestInfo Params) {
	This->ServerHandlePickupInfo_Implementation(Pickup, Params);
}

void AFortPlayerPawn::Init() {
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerPawn>(510, ServerHandlePickupInfoHook);
}

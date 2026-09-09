#include "pch.h"

void UFortGadgetItemDefinition::ApplyGadgetDataHook(const UFortGadgetItemDefinition* This, IFortInventoryOwnerInterface* InventoryOwner, UFortItem* Item, bool bLoadedFromRecord) {
	ApplyGadgetDataOG(This, InventoryOwner, Item, bLoadedFromRecord);

	if (!InventoryOwner) {
		return;
	}

	UObject* OwnerObject = InventoryOwner->_getUObject();
	AFortPlayerControllerZone* FortPC = OwnerObject ? OwnerObject->Cast<AFortPlayerControllerZone>() : nullptr;
	if (FortPC && FortPC->Role == ENetRole::ROLE_Authority && This->bDropAllOnEquip) {
		const UFortItemDefinition* IgnoreItemDef = Item ? Item->GetItemDefinition() : nullptr;
		FortPC->DropAllItems(IgnoreItemDef, nullptr, false, true);
	}
}

void UFortGadgetItemDefinition::Init() {
	Memory::HookDetour(ImageBase + 0x4A3A0B0, ApplyGadgetDataHook, &ApplyGadgetDataOG);
}

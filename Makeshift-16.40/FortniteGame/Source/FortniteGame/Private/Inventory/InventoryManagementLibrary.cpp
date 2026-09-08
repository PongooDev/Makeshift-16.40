#include "pch.h"

static AFortPlayerController* GetAuthorityPlayerController(const TScriptInterface<IFortInventoryOwnerInterface>& InventoryOwner) {
	UObject* InventoryOwnerObject = InventoryOwner.GetObject();
	IFortInventoryOwnerInterface* InventoryOwnerInterface = static_cast<IFortInventoryOwnerInterface*>(InventoryOwner.GetInterface());
	if (!InventoryOwnerObject || !InventoryOwnerInterface) {
		return nullptr;
	}

	if (InventoryOwnerInterface->GetNetworkRole() != ENetRole::ROLE_Authority) {
		return nullptr;
	}

	return InventoryOwnerObject->Cast<AFortPlayerController>();
}

UFortWorldItem* UInventoryManagementLibrary::AddItem(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const UFortItemDefinition* ItemDefinition, int32 Count) {
	AFortPlayerController* PlayerController = GetAuthorityPlayerController(InventoryOwner);
	if (!PlayerController || !ItemDefinition || Count <= 0) {
		return nullptr;
	}

	FFortItemEntry ItemEntry(ItemDefinition, Count, INDEX_NONE);
	return PlayerController->AddInventoryItem(ItemEntry, false);
}

TArray<UFortWorldItem*> UInventoryManagementLibrary::AddItems(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const TArray<FItemAndCount>& Items) {
	TArray<UFortWorldItem*> AddedItems;
	for (int32 Index = 0; Index < Items.Num(); ++Index) {
		UFortWorldItem* AddedItem = AddItem(InventoryOwner, Items[Index].Item, Items[Index].Count);
		if (AddedItem) {
			AddedItems.Add(AddedItem);
		}
	}

	return AddedItems;
}

bool UInventoryManagementLibrary::GiveItemEntryToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const FFortItemEntry& ItemInstance) {
	AFortPlayerController* PlayerController = GetAuthorityPlayerController(InventoryOwner);
	if (!PlayerController) {
		return false;
	}

	return PlayerController->AddInventoryItem(ItemInstance, false) != nullptr;
}

bool UInventoryManagementLibrary::RemoveItem(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const FGuid& ItemGuid, int32 Count) {
	AFortPlayerController* PlayerController = GetAuthorityPlayerController(InventoryOwner);
	if (!PlayerController) {
		return false;
	}

	return PlayerController->RemoveInventoryItem(ItemGuid, Count, false) > 0;
}

void UInventoryManagementLibrary::RemoveItems(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, const TArray<FItemGuidAndCount>& Items) {
	for (int32 Index = 0; Index < Items.Num(); ++Index) {
		RemoveItem(InventoryOwner, Items[Index].ItemGuid, Items[Index].Count);
	}
}

DEFINE_FUNCTION(UInventoryManagementLibrary::execAddItem)
{
	P_GET_TINTERFACE(IFortInventoryOwnerInterface,Z_Param_InventoryOwner);
	P_GET_OBJECT(UFortItemDefinition,Z_Param_ItemDefinition);
	P_GET_PROPERTY(FIntProperty,Z_Param_Count);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(UFortWorldItem**)Z_Param__Result=UInventoryManagementLibrary::AddItem(Z_Param_InventoryOwner,Z_Param_ItemDefinition,Z_Param_Count);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UInventoryManagementLibrary::execAddItems)
{
	P_GET_TINTERFACE(IFortInventoryOwnerInterface,Z_Param_InventoryOwner);
	P_GET_TARRAY_REF(FItemAndCount,Z_Param_Out_Items);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(TArray<UFortWorldItem*>*)Z_Param__Result=UInventoryManagementLibrary::AddItems(Z_Param_InventoryOwner,Z_Param_Out_Items);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UInventoryManagementLibrary::execGiveItemEntryToInventoryOwner)
{
	P_GET_TINTERFACE(IFortInventoryOwnerInterface,Z_Param_InventoryOwner);
	P_GET_STRUCT_REF(FFortItemEntry,Z_Param_Out_ItemInstance);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UInventoryManagementLibrary::GiveItemEntryToInventoryOwner(Z_Param_InventoryOwner,Z_Param_Out_ItemInstance);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UInventoryManagementLibrary::execRemoveItem)
{
	P_GET_TINTERFACE(IFortInventoryOwnerInterface,Z_Param_InventoryOwner);
	P_GET_STRUCT_REF(FGuid,Z_Param_Out_ItemGuid);
	P_GET_PROPERTY(FIntProperty,Z_Param_Count);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(bool*)Z_Param__Result=UInventoryManagementLibrary::RemoveItem(Z_Param_InventoryOwner,Z_Param_Out_ItemGuid,Z_Param_Count);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UInventoryManagementLibrary::execRemoveItems)
{
	P_GET_TINTERFACE(IFortInventoryOwnerInterface,Z_Param_InventoryOwner);
	P_GET_TARRAY_REF(FItemGuidAndCount,Z_Param_Out_Items);
	P_FINISH;
	P_NATIVE_BEGIN;
	UInventoryManagementLibrary::RemoveItems(Z_Param_InventoryOwner,Z_Param_Out_Items);
	P_NATIVE_END;
}

void UInventoryManagementLibrary::Init() {
	Memory::HookDetour(ImageBase + 0x51E5A44, execAddItem, nullptr);
	Memory::HookDetour(ImageBase + 0x51E5B8C, execAddItems, nullptr);
	Memory::HookDetour(ImageBase + 0x51E63DC, execGiveItemEntryToInventoryOwner, nullptr);
	Memory::HookDetour(ImageBase + 0x51E6C3C, execRemoveItem, nullptr);
	Memory::HookDetour(ImageBase + 0x51E6D94, execRemoveItems, nullptr);
}

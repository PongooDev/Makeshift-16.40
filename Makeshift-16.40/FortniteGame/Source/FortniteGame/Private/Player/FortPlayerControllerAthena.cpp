#include "pch.h"

static UAthenaPickaxeItemDefinition* LoadDefaultPickaxe() {
	TSoftObjectPtr<UObject> DefaultPickaxe = UKismetSystemLibrary::Conv_SoftObjPathToSoftObjRef(UKismetSystemLibrary::MakeSoftObjectPath(FString(L"/Game/Athena/Items/Cosmetics/Pickaxes/DefaultPickaxe.DefaultPickaxe")));
	UObject* LoadedPickaxe = UKismetSystemLibrary::LoadAsset_Blocking(DefaultPickaxe);
	return LoadedPickaxe ? LoadedPickaxe->Cast<UAthenaPickaxeItemDefinition>() : nullptr;
}

static UFortAbilitySet* LoadPlayerAbilitySet(const UGameDataBR* GameDataBR) {
	if (!GameDataBR) {
		return nullptr;
	}

	if (UFortAbilitySet* LoadedAbilitySet = GameDataBR->PlayerAbilitySetBR.Get()) {
		return LoadedAbilitySet;
	}

	TSoftObjectPtr<UObject> AbilitySetToLoad;
	static_cast<FSoftObjectPtr&>(AbilitySetToLoad) = GameDataBR->PlayerAbilitySetBR;
	UObject* LoadedAbilitySet = UKismetSystemLibrary::LoadAsset_Blocking(AbilitySetToLoad);
	return LoadedAbilitySet ? LoadedAbilitySet->Cast<UFortAbilitySet>() : nullptr;
}

static void GrantStartingItems(AFortPlayerControllerAthena* PlayerController, const TArray<FItemAndCount>& StartingItems) {
	for (int32 Index = 0; Index < StartingItems.Num(); ++Index) {
		const FItemAndCount& ItemAndCount = StartingItems[Index];
		if (ItemAndCount.Item && ItemAndCount.Count > 0) {
			PlayerController->AddInventoryItem(FFortItemEntry(ItemAndCount.Item, ItemAndCount.Count, 0), false);
		}
	}
}

void AFortPlayerControllerAthena::ServerAcknowledgePossession_Implementation(APawn* P) {
	Super::ServerAcknowledgePossession_Implementation(P);

	AFortPlayerPawn* PlayerPawn = P ? P->Cast<AFortPlayerPawn>() : nullptr;
	if (!PlayerPawn) {
		return;
	}

	UWorld* World = GetWorld();
	AFortGameStateAthena* GameState = World && World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
	const UFortPlaylistAthena* Playlist = nullptr;
	if (GameState) {
		Playlist = GameState->CurrentPlaylistInfo.OverridePlaylist ? GameState->CurrentPlaylistInfo.OverridePlaylist : GameState->CurrentPlaylistInfo.BasePlaylist;
	}

	if (WorldInventory && !bHasInitializedWorldInventory) {
		AFortGameModeAthena* GameMode = World && World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameModeAthena>() : nullptr;
		if (GameMode && !(Playlist && Playlist->bIgnoreGameModeStartingInventory)) {
			GrantStartingItems(this, GameMode->GetStartingItems(true, this));
		}

		if (Playlist) {
			GrantStartingItems(this, Playlist->InventoryItemsToGrant);
		}

		if (!Playlist || Playlist->bRequirePickaxeInStartingInventory) {
			UAthenaPickaxeItemDefinition* PickaxeDefinition = CosmeticLoadoutPC.Pickaxe ? CosmeticLoadoutPC.Pickaxe : PlayerPawn->CosmeticLoadout.Pickaxe;
			if (!PickaxeDefinition) {
				PickaxeDefinition = LoadDefaultPickaxe();
			}

			if (PickaxeDefinition && PickaxeDefinition->WeaponDefinition) {
				AddInventoryItem(FFortItemEntry(PickaxeDefinition->WeaponDefinition, 1, 0), false);
			}
		}

		bHasInitializedWorldInventory = true;
	}

	if (UFortAbilitySet* PlayerAbilitySet = LoadPlayerAbilitySet(UGameDataBR::Get())) {
		TScriptInterface<IAbilitySystemInterface> AbilitySystemInterface;
		AbilitySystemInterface.SetObject(PlayerPawn);
		AbilitySystemInterface.SetInterface(PlayerPawn->GetInterfaceAddress(IAbilitySystemInterface::StaticClass()));
		UFortKismetLibrary::EquipFortAbilitySet(AbilitySystemInterface, PlayerAbilitySet, nullptr);
	}
}

void AFortPlayerControllerAthena::ServerAcknowledgePossessionHook(AFortPlayerControllerAthena* This, APawn* P) {
	This->ServerAcknowledgePossession_Implementation(P);
}

void AFortPlayerControllerAthena::Init() {
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerControllerAthena>(275, ServerAcknowledgePossessionHook);
}

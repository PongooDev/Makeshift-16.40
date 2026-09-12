#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/DataTable.h"

int32 UFortAthenaAISpawnerDataComponent_CosmeticLibrary::GetAICosmeticLibraryDataIndexHook(const UFortAthenaAISpawnerDataComponent_CosmeticLibrary* This)
{
	const int32 NumLibraries = This->CosmeticLibraries.Num();

	float TotalWeight = 0.f;
	for (int32 LibraryIndex = 0; LibraryIndex < NumLibraries; ++LibraryIndex)
	{
		TotalWeight += FMath::Max(This->DefaultCosmeticLibraryWeight.GetValueAtLevel(static_cast<float>(LibraryIndex)), 0.f);
	}

	if (TotalWeight <= 0.f)
	{
		return INDEX_NONE;
	}

	const float RandomWeight = FMath::FRand() * TotalWeight;
	float AccumulatedWeight = 0.f;
	for (int32 LibraryIndex = 0; LibraryIndex < NumLibraries; ++LibraryIndex)
	{
		const float LibraryWeight = This->DefaultCosmeticLibraryWeight.GetValueAtLevel(static_cast<float>(LibraryIndex));
		if (LibraryWeight <= 0.f)
		{
			continue;
		}

		AccumulatedWeight += LibraryWeight;
		if (RandomWeight < AccumulatedWeight)
		{
			return LibraryIndex;
		}
	}

	return NumLibraries - 1;
}

UFortAthenaAIBotCosmeticLibraryData* UFortAthenaAISpawnerDataComponent_CosmeticLibrary::GetAICosmeticLibraryDataHook(const UFortAthenaAISpawnerDataComponent_CosmeticLibrary* This)
{
	const int32 LibraryIndex = This->GetAICosmeticLibraryDataIndex();
	if (LibraryIndex < 0 || LibraryIndex >= This->CosmeticLibraries.Num())
	{
		return nullptr;
	}

	UObject* LoadedLibrary = LoadSoftObject(This->CosmeticLibraries[LibraryIndex]);
	return LoadedLibrary ? LoadedLibrary->Cast<UFortAthenaAIBotCosmeticLibraryData>() : nullptr;
}

UObject* UFortAthenaAISpawnerDataComponent_CosmeticLibrary::LoadSoftObject(const FSoftObjectPtr& SoftObject)
{
	if (SoftObject.ObjectID.AssetPathName == FName())
	{
		return nullptr;
	}

	if (UObject* AlreadyLoaded = SoftObject.Get())
	{
		return AlreadyLoaded;
	}

	FSoftObjectPath SoftObjectPath{};
	SoftObjectPath.AssetPathName = SoftObject.ObjectID.AssetPathName;
	SoftObjectPath.SubPathString = SoftObject.ObjectID.SubPathString;

	return UKismetSystemLibrary::LoadAsset_Blocking(UKismetSystemLibrary::Conv_SoftObjPathToSoftObjRef(SoftObjectPath));
}

UDataTable* UFortAthenaAISpawnerDataComponent_CosmeticLibrary::LoadCosmeticDataTable(const TSoftObjectPtr<UDataTable>& DataTable)
{
	struct FResolvedDataTable
	{
		FName AssetPathName;
		UDataTable* Table;
	};
	static TArray<FResolvedDataTable> ResolvedDataTables;

	const FName AssetPathName = DataTable.ObjectID.AssetPathName;
	if (AssetPathName == FName())
	{
		return nullptr;
	}

	for (int32 ResolvedIndex = 0; ResolvedIndex < ResolvedDataTables.Num(); ++ResolvedIndex)
	{
		if (ResolvedDataTables[ResolvedIndex].AssetPathName == AssetPathName)
		{
			return ResolvedDataTables[ResolvedIndex].Table;
		}
	}

	UObject* LoadedObject = LoadSoftObject(DataTable);
	UDataTable* Table = LoadedObject ? LoadedObject->Cast<UDataTable>() : nullptr;
	if (Table == nullptr)
	{
		UE_LOG(LogAISpawnerData, Log, TEXT("[%hs] the bot cosmetic data table %hs is not available in this build, falling back on the cosmetic item definitions"), __FUNCTION__, AssetPathName.ToString().c_str());
	}

	FResolvedDataTable ResolvedDataTable{};
	ResolvedDataTable.AssetPathName = AssetPathName;
	ResolvedDataTable.Table = Table;
	ResolvedDataTables.Add(ResolvedDataTable);

	return Table;
}

int32 UFortAthenaAISpawnerDataComponent_CosmeticLibrary::GetRandomEmoteDataTableRowIndex(const TArray<const FFortBotCosmeticItemDataTableRow*>& DataTableRows, float TotalWeight)
{
	if (TotalWeight <= 0.f)
	{
		return INDEX_NONE;
	}

	const float RandomWeight = FMath::FRand() * TotalWeight;
	float AccumulatedWeight = 0.f;
	for (int32 RowIndex = 0; RowIndex < DataTableRows.Num(); ++RowIndex)
	{
		const FFortBotCosmeticItemDataTableRow* DataTableRow = DataTableRows[RowIndex];
		if (DataTableRow == nullptr || DataTableRow->Weight <= 0.f)
		{
			continue;
		}

		AccumulatedWeight += DataTableRow->Weight;
		if (RandomWeight < AccumulatedWeight)
		{
			return RowIndex;
		}
	}

	return INDEX_NONE;
}

UFortItemDefinition* UFortAthenaAISpawnerDataComponent_CosmeticLibrary::PickItemFromDataTable(const TSoftObjectPtr<UDataTable>& DataTable)
{
	UDataTable* Table = LoadCosmeticDataTable(DataTable);
	if (Table == nullptr)
	{
		return nullptr;
	}

	TArray<FFortBotCosmeticItemDataTableRow*> AllRows;
	Table->GetAllRows<FFortBotCosmeticItemDataTableRow>(TEXT("UFortAthenaAISpawnerDataComponent_CosmeticLibrary::PickItemFromDataTable"), AllRows);

	TArray<const FFortBotCosmeticItemDataTableRow*> DataTableRows;
	float TotalWeight = 0.f;
	for (int32 RowIndex = 0; RowIndex < AllRows.Num(); ++RowIndex)
	{
		const FFortBotCosmeticItemDataTableRow* DataTableRow = AllRows[RowIndex];
		if (DataTableRow == nullptr || DataTableRow->Weight <= 0.f || !DataTableRow->PrimaryAssetId.IsValid())
		{
			continue;
		}

		DataTableRows.Add(DataTableRow);
		TotalWeight += DataTableRow->Weight;
	}

	const int32 PickedRowIndex = GetRandomEmoteDataTableRowIndex(DataTableRows, TotalWeight);
	if (PickedRowIndex == INDEX_NONE)
	{
		return nullptr;
	}

	UObject* LoadedItem = UKismetSystemLibrary::LoadAsset_Blocking(UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(DataTableRows[PickedRowIndex]->PrimaryAssetId));
	return LoadedItem ? LoadedItem->Cast<UFortItemDefinition>() : nullptr;
}

UFortItemDefinition* UFortAthenaAISpawnerDataComponent_CosmeticLibrary::PickItemFromPrimaryAssets(const FName& PrimaryAssetTypeName, TArray<FPrimaryAssetId>& CachedPrimaryAssetIds, UClass* ItemClass)
{
	const EFortRarity MaxRarity = EFortRarity::Rare;
	const int32 MaxAttempts = 16;

	if (CachedPrimaryAssetIds.Num() == 0)
	{
		FPrimaryAssetType PrimaryAssetType{};
		PrimaryAssetType.Name = PrimaryAssetTypeName;
		UKismetSystemLibrary::GetPrimaryAssetIdList(PrimaryAssetType, &CachedPrimaryAssetIds);

		UE_LOG(LogAISpawnerData, Log, TEXT("[%hs] %d %hs primary assets available for the bot cosmetic loadouts"), __FUNCTION__, CachedPrimaryAssetIds.Num(), PrimaryAssetTypeName.ToString().c_str());
	}

	const int32 NumPrimaryAssets = CachedPrimaryAssetIds.Num();
	if (NumPrimaryAssets <= 0)
	{
		return nullptr;
	}

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		const FPrimaryAssetId& PrimaryAssetId = CachedPrimaryAssetIds[FMath::RandHelper(NumPrimaryAssets)];

		UObject* LoadedItem = UKismetSystemLibrary::LoadAsset_Blocking(UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(PrimaryAssetId));
		UFortItemDefinition* ItemDefinition = LoadedItem ? LoadedItem->Cast<UFortItemDefinition>() : nullptr;
		if (ItemDefinition == nullptr || !ItemDefinition->IsA(ItemClass))
		{
			continue;
		}

		if (ItemDefinition->Rarity > MaxRarity)
		{
			continue;
		}

		return ItemDefinition;
	}

	return nullptr;
}

UFortItemDefinition* UFortAthenaAISpawnerDataComponent_CosmeticLibrary::PickCosmeticItem(const TSoftObjectPtr<UDataTable>& DataTable, const FName& PrimaryAssetTypeName, TArray<FPrimaryAssetId>& CachedPrimaryAssetIds, UClass* ItemClass)
{
	UFortItemDefinition* ItemDefinition = PickItemFromDataTable(DataTable);
	if (ItemDefinition && ItemDefinition->IsA(ItemClass))
	{
		return ItemDefinition;
	}

	return PickItemFromPrimaryAssets(PrimaryAssetTypeName, CachedPrimaryAssetIds, ItemClass);
}

void UFortAthenaAISpawnerDataComponent_CosmeticLibrary::GetDancesHook(UFortAthenaAISpawnerDataComponent_CosmeticLibrary* This, TArray<UAthenaDanceItemDefinition*>* Dances)
{
	if (This == nullptr || Dances == nullptr)
	{
		return;
	}

	static TArray<FPrimaryAssetId> DancePrimaryAssetIds;

	const int32 EmotesMaxCount = This->EmotesMaxCount.AsInteger(0.f);
	for (int32 EmoteIndex = 0; EmoteIndex < EmotesMaxCount; ++EmoteIndex)
	{
		UFortItemDefinition* ItemDefinition = PickCosmeticItem(This->EmotesDataTable, FName(L"AthenaDance"), DancePrimaryAssetIds, UAthenaDanceItemDefinition::StaticClass());
		UAthenaDanceItemDefinition* Dance = ItemDefinition ? ItemDefinition->Cast<UAthenaDanceItemDefinition>() : nullptr;
		if (Dance == nullptr)
		{
			continue;
		}

		bool bAlreadyPicked = false;
		for (int32 DanceIndex = 0; DanceIndex < Dances->Num(); ++DanceIndex)
		{
			if ((*Dances)[DanceIndex] == Dance)
			{
				bAlreadyPicked = true;
				break;
			}
		}

		if (!bAlreadyPicked)
		{
			Dances->Add(Dance);
		}
	}

	UE_LOG(LogAISpawnerData, Verbose, TEXT("[%hs] on %hs picked %d dances out of %d"), __FUNCTION__, This->GetName().c_str(), Dances->Num(), EmotesMaxCount);
}

void UFortAthenaAISpawnerDataComponent_CosmeticLibrary::GetLoadoutHook(UFortAthenaAISpawnerDataComponent_CosmeticLibrary* This, FFortAthenaLoadout* OutLoadout)
{
	if (This == nullptr || OutLoadout == nullptr)
	{
		return;
	}

	static TArray<FPrimaryAssetId> CharacterPrimaryAssetIds;
	static TArray<FPrimaryAssetId> GliderPrimaryAssetIds;
	static TArray<FPrimaryAssetId> SkyDiveContrailPrimaryAssetIds;
	static TArray<FPrimaryAssetId> PickaxePrimaryAssetIds;

	const int32 MaxCharacterAttempts = 16;

	const UGameDataCosmetics& GameDataCosmetics = UGameDataCosmetics::Get();
	UFortAthenaAIBotCosmeticLibraryData* CosmeticLibrary = This->GetAICosmeticLibraryData();

	UAthenaCharacterItemDefinition* Character = nullptr;
	if (CosmeticLibrary)
	{
		UFortItemDefinition* ItemDefinition = PickItemFromDataTable(CosmeticLibrary->CharactersDataTable);
		if (ItemDefinition == nullptr)
		{
			ItemDefinition = PickItemFromDataTable(CosmeticLibrary->FallbackCharactersDataTable);
		}
		Character = ItemDefinition ? ItemDefinition->Cast<UAthenaCharacterItemDefinition>() : nullptr;
	}

	for (int32 Attempt = 0; Character == nullptr && Attempt < MaxCharacterAttempts; ++Attempt)
	{
		UFortItemDefinition* ItemDefinition = PickItemFromPrimaryAssets(FName(L"AthenaCharacter"), CharacterPrimaryAssetIds, UAthenaCharacterItemDefinition::StaticClass());
		UAthenaCharacterItemDefinition* Candidate = ItemDefinition ? ItemDefinition->Cast<UAthenaCharacterItemDefinition>() : nullptr;
		if (Candidate == nullptr || Candidate->HeroDefinition == nullptr)
		{
			continue;
		}

		bool bIsDefaultCharacter = false;
		for (int32 DefaultIndex = 0; DefaultIndex < GameDataCosmetics.RandomCharacters.Num(); ++DefaultIndex)
		{
			if (GameDataCosmetics.RandomCharacters[DefaultIndex] == Candidate)
			{
				bIsDefaultCharacter = true;
				break;
			}
		}

		if (!bIsDefaultCharacter)
		{
			Character = Candidate;
		}
	}

	if (Character == nullptr && GameDataCosmetics.RandomCharacters.Num() > 0)
	{
		Character = GameDataCosmetics.RandomCharacters[FMath::RandHelper(GameDataCosmetics.RandomCharacters.Num())];
	}

	if (Character == nullptr)
	{
		UE_LOG(LogAISpawnerData, Warning, TEXT("[%hs] on %hs... there is no Character to pick from!"), __FUNCTION__, This->GetName().c_str());
		return;
	}

	OutLoadout->Character = Character;
	OutLoadout->Backpack = Character->DefaultBackpack;

	UFortItemDefinition* Glider = CosmeticLibrary ? PickCosmeticItem(CosmeticLibrary->GlidersDataTable, FName(L"AthenaGlider"), GliderPrimaryAssetIds, UAthenaGliderItemDefinition::StaticClass()) : PickItemFromPrimaryAssets(FName(L"AthenaGlider"), GliderPrimaryAssetIds, UAthenaGliderItemDefinition::StaticClass());
	OutLoadout->Glider = Glider ? Glider->Cast<UAthenaGliderItemDefinition>() : nullptr;
	if (OutLoadout->Glider == nullptr)
	{
		OutLoadout->Glider = GameDataCosmetics.DefaultGliderSkin;
	}

	UFortItemDefinition* SkyDiveContrail = CosmeticLibrary ? PickCosmeticItem(CosmeticLibrary->SkyDiveContrailsDataTable, FName(L"AthenaSkyDiveContrail"), SkyDiveContrailPrimaryAssetIds, UAthenaSkyDiveContrailItemDefinition::StaticClass()) : PickItemFromPrimaryAssets(FName(L"AthenaSkyDiveContrail"), SkyDiveContrailPrimaryAssetIds, UAthenaSkyDiveContrailItemDefinition::StaticClass());
	OutLoadout->SkyDiveContrail = SkyDiveContrail ? SkyDiveContrail->Cast<UAthenaSkyDiveContrailItemDefinition>() : nullptr;
	if (OutLoadout->SkyDiveContrail == nullptr)
	{
		OutLoadout->SkyDiveContrail = GameDataCosmetics.DefaultContrailEffect;
	}

	UFortItemDefinition* Pickaxe = CosmeticLibrary ? PickCosmeticItem(CosmeticLibrary->PickaxesDataTable, FName(L"AthenaPickaxe"), PickaxePrimaryAssetIds, UAthenaPickaxeItemDefinition::StaticClass()) : PickItemFromPrimaryAssets(FName(L"AthenaPickaxe"), PickaxePrimaryAssetIds, UAthenaPickaxeItemDefinition::StaticClass());
	OutLoadout->Pickaxe = Pickaxe ? Pickaxe->Cast<UAthenaPickaxeItemDefinition>() : nullptr;
	if (OutLoadout->Pickaxe == nullptr)
	{
		UObject* FallbackPickaxe = LoadSoftObject(GameDataCosmetics.FallbackPickaxe);
		OutLoadout->Pickaxe = FallbackPickaxe ? FallbackPickaxe->Cast<UAthenaPickaxeItemDefinition>() : nullptr;
	}

	UE_LOG(LogAISpawnerData, Verbose, TEXT("[%hs] on %hs picked Character %hs Backpack %hs Glider %hs SkyDiveContrail %hs Pickaxe %hs from library %hs"), __FUNCTION__, This->GetName().c_str(),
		OutLoadout->Character->GetName().c_str(),
		OutLoadout->Backpack ? OutLoadout->Backpack->GetName().c_str() : "nothing",
		OutLoadout->Glider ? OutLoadout->Glider->GetName().c_str() : "nothing",
		OutLoadout->SkyDiveContrail ? OutLoadout->SkyDiveContrail->GetName().c_str() : "nothing",
		OutLoadout->Pickaxe ? OutLoadout->Pickaxe->GetName().c_str() : "nothing",
		CosmeticLibrary ? CosmeticLibrary->GetName().c_str() : "nothing");
}

void UFortAthenaAISpawnerDataComponent_CosmeticLibrary::Init()
{
	Memory::SwapVTableEntryInAllSubClasses<UFortAthenaAISpawnerDataComponent_CosmeticLibrary>(83, GetDancesHook);
	Memory::SwapVTableEntryInAllSubClasses<UFortAthenaAISpawnerDataComponent_CosmeticLibrary>(84, GetLoadoutHook);
	Memory::SwapVTableEntryInAllSubClasses<UFortAthenaAISpawnerDataComponent_CosmeticLibrary>(85, GetAICosmeticLibraryDataIndexHook);
	Memory::SwapVTableEntryInAllSubClasses<UFortAthenaAISpawnerDataComponent_CosmeticLibrary>(86, GetAICosmeticLibraryDataHook);
}

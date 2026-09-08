#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/FortAssets.h"

void UGameDataBR::GetLootTierDataTablesBR(TArray<UDataTable*>& OutArray) const {
	FFortAssets::GetAssetArray<UDataTable>(LootTierDataTablesBR, OutArray);
}

void UGameDataBR::GetLootPackageDataTablesBR(TArray<UDataTable*>& OutArray) const {
	FFortAssets::GetAssetArray<UDataTable>(LootPackageDataTablesBR, OutArray);
}

const UGameDataBR* UGameDataBR::Get() {
	return UFortAssetManager::Get().GetGameDataBR();
}

UFortItemVariantData* UGameDataBR::GetItemVariantDataFromItemDefTags(const FGameplayTagContainer& ItemDefTags, FGameplayTag& OutItemVariantTag, int32& OutItemVariantDataMappingIndex) const {
	const UFortItemDefToItemVariantDataMapping* ItemDefToItemVariantDataMapping = FFortAssets::GetAsset(ItemDefToItemVariantDataMappingAsset);
	if (!ItemDefToItemVariantDataMapping) {
		return nullptr;
	}

	for (int32 MappingIndex = 0; MappingIndex < ItemDefToItemVariantDataMapping->ItemDefToItemVariantDataMappings.Num(); ++MappingIndex) {
		const FItemDefToItemVariantDataMapping& Mapping = ItemDefToItemVariantDataMapping->ItemDefToItemVariantDataMappings[MappingIndex];
		for (const FGameplayTag& ItemDefinitionTag : Mapping.ItemDefinitionTags.GameplayTags) {
			if (ItemDefTags.HasTag(ItemDefinitionTag)) {
				OutItemVariantTag = Mapping.ItemVariantTag;
				OutItemVariantDataMappingIndex = MappingIndex;
				return Mapping.ItemVariantData;
			}
		}
	}

	return nullptr;
}

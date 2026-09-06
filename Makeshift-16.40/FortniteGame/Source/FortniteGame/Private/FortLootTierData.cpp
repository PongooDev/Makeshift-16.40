#include "pch.h"

static float GetLootChanceMultiplier(UWorld* World, const FGameplayTagContainer& GameplayTags) {
	const AFortGameMode* GameMode = World && World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameMode>() : nullptr;
	return GameMode ? GameMode->GetLootChanceMultiplier(GameplayTags) : 1.f;
}

static float GetLootCountMultiplier(UWorld* World, const FGameplayTagContainer& GameplayTags) {
	const AFortGameMode* GameMode = World && World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameMode>() : nullptr;
	return GameMode ? GameMode->GetLootCountMultiplier(GameplayTags) : 1.f;
}

float FFortLootTierData::GetTrueWeight(UWorld* World, const TMap<FName, float>* NamedWeightsMap) const {
	return GetLootChanceMultiplier(World, GameplayTags) * Weight;
}

float FFortLootPackageData::GetTrueWeight(UWorld* World, const TMap<FName, float>* NamedWeightsMap) const {
	const float LootChanceMultiplier = GetLootChanceMultiplier(World, GameplayTags);

	float NamedWeight = 0.f;
	if (NamedWeightsMap) {
		for (const FName& PotentialNamedWeight : PotentialNamedWeights) {
			if (const float* FoundNamedWeight = NamedWeightsMap->Find(PotentialNamedWeight)) {
				NamedWeight += *FoundNamedWeight;
			}
		}
	}

	return FMath::Max((NamedWeight + Weight) * LootChanceMultiplier, 0.f);
}

float FFortLootPackageData::GetTrueCount(UWorld* World) const {
	return FMath::CeilToFloat(static_cast<float>(Count) * GetLootCountMultiplier(World, GameplayTags));
}

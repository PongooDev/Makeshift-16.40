#include "pch.h"

struct FFortGlobalGameplayTags
{
	uint8 Pad_0[0x390];
	FGameplayTag DamageFalling;
	uint8 Pad_398[0x470 - 0x398];
	FGameplayTag EffectInstantDeathStructuralSupport;
	FGameplayTag EffectInstantDeathSilent;
	uint8 Pad_480[0x750 - 0x480];
	FGameplayTag ActionPlayerInteractBuild;
	uint8 Pad_758[0xCA0 - 0x758];
	FGameplayTag WeaponKeepsMaterialsOnDropAll;
	uint8 Pad_CA8[0xE80 - 0xCA8];
	FGameplayTag BuildingContainer_TreasureChest;
	FGameplayTag BuildingContainer_AmmoBox;
	uint8 Pad_E90[0xFB8 - 0xE90];
	FGameplayTag PlayerDisconnected;
};
static_assert(offsetof(FFortGlobalGameplayTags, DamageFalling) == 0x000390, "Member 'FFortGlobalGameplayTags::DamageFalling' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, EffectInstantDeathStructuralSupport) == 0x000470, "Member 'FFortGlobalGameplayTags::EffectInstantDeathStructuralSupport' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, EffectInstantDeathSilent) == 0x000478, "Member 'FFortGlobalGameplayTags::EffectInstantDeathSilent' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, ActionPlayerInteractBuild) == 0x000750, "Member 'FFortGlobalGameplayTags::ActionPlayerInteractBuild' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, WeaponKeepsMaterialsOnDropAll) == 0x000CA0, "Member 'FFortGlobalGameplayTags::WeaponKeepsMaterialsOnDropAll' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, BuildingContainer_TreasureChest) == 0x000E80, "Member 'FFortGlobalGameplayTags::BuildingContainer_TreasureChest' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, BuildingContainer_AmmoBox) == 0x000E88, "Member 'FFortGlobalGameplayTags::BuildingContainer_AmmoBox' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, PlayerDisconnected) == 0x000FB8, "Member 'FFortGlobalGameplayTags::PlayerDisconnected' has a wrong offset!");

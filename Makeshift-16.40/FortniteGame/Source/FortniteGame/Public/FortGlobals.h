#include "pch.h"

struct FFortGlobalGameplayTags
{
	uint8 Pad_0[0x390];
	FGameplayTag DamageFalling;
	uint8 Pad_398[0x470 - 0x398];
	FGameplayTag EffectInstantDeathStructuralSupport;
	uint8 Pad_478[0x750 - 0x478];
	FGameplayTag ActionPlayerInteractBuild;
	uint8 Pad_758[0xCA0 - 0x758];
	FGameplayTag WeaponKeepsMaterialsOnDropAll;
	uint8 Pad_CA8[0xFB8 - 0xCA8];
	FGameplayTag PlayerDisconnected;
};
static_assert(offsetof(FFortGlobalGameplayTags, DamageFalling) == 0x000390, "Member 'FFortGlobalGameplayTags::DamageFalling' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, EffectInstantDeathStructuralSupport) == 0x000470, "Member 'FFortGlobalGameplayTags::EffectInstantDeathStructuralSupport' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, ActionPlayerInteractBuild) == 0x000750, "Member 'FFortGlobalGameplayTags::ActionPlayerInteractBuild' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, WeaponKeepsMaterialsOnDropAll) == 0x000CA0, "Member 'FFortGlobalGameplayTags::WeaponKeepsMaterialsOnDropAll' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, PlayerDisconnected) == 0x000FB8, "Member 'FFortGlobalGameplayTags::PlayerDisconnected' has a wrong offset!");

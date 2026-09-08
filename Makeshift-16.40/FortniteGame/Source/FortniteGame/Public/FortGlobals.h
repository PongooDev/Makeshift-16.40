#include "pch.h"

struct FFortGlobalGameplayTags
{
	uint8 Pad_0[0x470];
	FGameplayTag EffectInstantDeathStructuralSupport;
	uint8 Pad_478[0x750 - 0x478];
	FGameplayTag ActionPlayerInteractBuild;
};
static_assert(offsetof(FFortGlobalGameplayTags, EffectInstantDeathStructuralSupport) == 0x000470, "Member 'FFortGlobalGameplayTags::EffectInstantDeathStructuralSupport' has a wrong offset!");
static_assert(offsetof(FFortGlobalGameplayTags, ActionPlayerInteractBuild) == 0x000750, "Member 'FFortGlobalGameplayTags::ActionPlayerInteractBuild' has a wrong offset!");

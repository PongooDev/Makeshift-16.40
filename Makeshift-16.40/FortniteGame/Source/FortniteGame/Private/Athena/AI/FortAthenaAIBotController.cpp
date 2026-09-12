#include "pch.h"

void AFortAthenaAIBotController::ApplyCharacterCustomizationHook(AFortAthenaAIBotController* This, const FFortAthenaLoadout* OverrideCosmeticLoadout)
{
	ApplyCharacterCustomizationOG(This, OverrideCosmeticLoadout);

	if (This->PlayerBotPawn == nullptr)
	{
		return;
	}

	AFortPlayerState* FortPlayerState = This->PlayerState ? This->PlayerState->Cast<AFortPlayerState>() : nullptr;
	if (FortPlayerState)
	{
		FortPlayerState->ApplyCharacterCustomization(This->PlayerBotPawn);
	}
}

void AFortAthenaAIBotController::Init()
{
	Memory::HookDetour(ImageBase + 0x4301B24, ApplyCharacterCustomizationHook, &ApplyCharacterCustomizationOG);
}

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

void AFortAthenaAIBotController::EnterAircraft(AFortAthenaAircraft* InAircraft)
{
	UFortControllerComponent_Aircraft* AircraftComponent = FindComponentByClass<UFortControllerComponent_Aircraft>();
	if (AircraftComponent && !AircraftComponent->CurrentAircraft && !AircraftComponent->bHasExitedAircraft)
	{
		AircraftComponent->EnterAircraft(InAircraft);
	}

	if (CachedBotManager)
	{
		UWorld* World = GetWorld();
		AFortGameStateAthena* GameState = World && World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
		if (GameState)
		{
			if (FMath::FRand() < CachedBotManager->ThankBusDriverProbability.GetValueAtLevel(0.f))
			{
				const float ThankBusDriverTime = FMath::FRandRange(CachedBotManager->ThankBusDriverMinTime.GetValueAtLevel(0.f), CachedBotManager->ThankBusDriverMaxTime.GetValueAtLevel(0.f));
				if (ThankBusDriverTime > 0.f)
				{
					ThankBusDriverTimerHandle = UKismetSystemLibrary::K2_SetTimer(this, FString(L"ThankBusDriver"), ThankBusDriverTime, false, 0.f, 0.f);
				}
			}
		}
	}

	if (Blackboard)
	{
		Blackboard->SetValue<UBlackboardKeyType_Bool>(IsInBusKey, true);
		Blackboard->SetValue<UBlackboardKeyType_Bool>(HasEverJumpedFromBusKey, false);

		if (HasEverJumpedFromBusAndLandedKey != 0xFF && Blackboard)
		{
			Blackboard->SetValue<UBlackboardKeyType_Bool>(HasEverJumpedFromBusAndLandedKey, false);
		}

		AFortPlayerStateAthena* PlayerStateAthena = PlayerState ? PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
		if (PlayerStateAthena)
		{
			PlayerStateAthena->SetHasEverSkydivedFromBusAndLanded(false);
		}
	}
}

void AFortAthenaAIBotController::Init()
{
	Memory::HookDetour(ImageBase + 0x4301B24, ApplyCharacterCustomizationHook, &ApplyCharacterCustomizationOG);
}

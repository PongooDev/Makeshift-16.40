#include "pch.h"

void UFortServerBotManagerAthena::CheckForBotBrainActivation()
{
	if (!CachedAIPopulationTracker)
	{
		UE_LOG(LogAthenaBots, Error, TEXT("No CachedAIPopulationTracker present!"));
		return;
	}

	bCanActivateBotsBrain = true;

	for (int32 Index = 0; Index < CachedAIPopulationTracker->PlayerBots.Num(); ++Index)
	{
		AFortAthenaAIBotController* BotController = CachedAIPopulationTracker->PlayerBots[Index].BotController;
		if (BotController)
		{
			BotController->RunBehaviorTree(BotController->BehaviorTree);
		}
	}

	for (int32 Index = 0; Index < CachedAIPopulationTracker->NoneAthenaParticipantBots.Num(); ++Index)
	{
		AFortAthenaAIBotController* BotController = CachedAIPopulationTracker->NoneAthenaParticipantBots[Index].BotController;
		if (BotController)
		{
			BotController->RunBehaviorTree(BotController->BehaviorTree);
		}
	}
}

void UFortServerBotManagerAthena::CacheValidPOIVolumesHook(UFortServerBotManagerAthena* This)
{
	CacheValidPOIVolumesOG(This);

	const int32 CachedVolumeCount = This->CachedValidPOIVolumeLocations.Num();
	for (int32 Index = CachedVolumeCount - 1; Index >= 0; --Index)
	{
		AFortPoiVolume* POIVolume = This->CachedValidPOIVolumeLocations[Index].POIVolume;
		if (!POIVolume || !POIVolume->RootComponent || !POIVolume->BrushComponent)
		{
			This->CachedValidPOIVolumeLocations.RemoveAt(Index, 1, false);
		}
	}

	const int32 DroppedVolumeCount = CachedVolumeCount - This->CachedValidPOIVolumeLocations.Num();
	if (DroppedVolumeCount > 0)
	{
		UE_LOG(LogAthenaBots, Warning, TEXT("UFortServerBotManagerAthena::CacheValidPOIVolumes : %d of the %d point of interest volume(s) have no brush in this build and would hand out a zero drop location, dropping them leaves %d"), DroppedVolumeCount, CachedVolumeCount, This->CachedValidPOIVolumeLocations.Num());
	}
}

void UFortServerBotManagerAthena::PlacePlayerBotsInAircraft(const TArray<AFortAthenaAircraft*>& Aircrafts)
{
	if (!CachedGameState)
	{
		return;
	}

	if (!CachedAIPopulationTracker)
	{
		UE_LOG(LogAthenaBots, Error, TEXT("[%hs] No CachedAIPopulationTracker present!"), __FUNCTION__);
		return;
	}

	for (int32 Index = 0; Index < CachedAIPopulationTracker->PlayerBots.Num(); ++Index)
	{
		AFortAthenaAIBotController* BotController = CachedAIPopulationTracker->PlayerBots[Index].BotController;
		if (!BotController)
		{
			continue;
		}

		for (int32 AircraftIndex = 0; AircraftIndex < Aircrafts.Num(); ++AircraftIndex)
		{
			if (CachedGameState->GetAircraftIndex(BotController->PlayerState) == AircraftIndex)
			{
				BotController->EnterAircraft(CachedGameState->GetAircraft(AircraftIndex));
				break;
			}
		}
	}
}

void UFortServerBotManagerAthena::ForceAllExitAircraft()
{
	if (!CachedAIPopulationTracker)
	{
		UE_LOG(LogAthenaBots, Error, TEXT("[%hs] No CachedAIPopulationTracker present!"), __FUNCTION__);
		return;
	}

	for (int32 Index = 0; Index < CachedAIPopulationTracker->PlayerBots.Num(); ++Index)
	{
		AFortAthenaAIBotController* BotController = CachedAIPopulationTracker->PlayerBots[Index].BotController;
		if (!BotController)
		{
			continue;
		}

		if (!BotController->PlayerBotPawn)
		{
			BotController->ExitAircraft();
		}
	}
}

void UFortServerBotManagerAthena::OnGamePhaseStepChanged(const TScriptInterface<IFortSafeZoneInterface>& SafeZoneInterface, const EAthenaGamePhaseStep GamePhaseStep)
{
	if (!CachedGameState)
	{
		return;
	}

	UE_LOG(LogAthenaBots, Log, TEXT("[%hs] Game phase step is now %d"), __FUNCTION__, static_cast<int32>(GamePhaseStep));

	if (GamePhaseStep == EAthenaGamePhaseStep::BusLocked)
	{
		PlacePlayerBotsInAircraft(CachedGameState->Aircrafts);
	}
	else if (GamePhaseStep != EAthenaGamePhaseStep::BusFlying)
	{
		ForceAllExitAircraft();
	}
}

DEFINE_FUNCTION(UFortServerBotManagerAthena::execOnGamePhaseStepChanged)
{
	P_GET_TINTERFACE_REF(IFortSafeZoneInterface,Z_Param_Out_SafeZoneInterface);
	P_GET_ENUM(EAthenaGamePhaseStep,Z_Param_GamePhaseStep);
	P_FINISH;
	P_THIS_CAST(UFortServerBotManagerAthena)->OnGamePhaseStepChanged(Z_Param_Out_SafeZoneInterface,EAthenaGamePhaseStep(Z_Param_GamePhaseStep));
}

void UFortServerBotManagerAthena::Init()
{
	Memory::HookDetour(ImageBase + 0x461496C, CacheValidPOIVolumesHook, &CacheValidPOIVolumesOG);
	Memory::HookDetour(ImageBase + 0x5194B50, (void*)&UFortServerBotManagerAthena::execOnGamePhaseStepChanged);
}

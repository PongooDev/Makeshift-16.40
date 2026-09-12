#include "pch.h"

bool AFortGameStateAthena::CanUpdateGamePhaseStepHook(AFortGameStateAthena* This)
{
	return true;
}

void AFortGameStateAthena::PlacePlayersInAircraftHook(AFortGameStateAthena* This)
{
	PlacePlayersInAircraftOG(This);

	UWorld* World = This->GetWorld();
	AFortGameModeAthena* GameMode = World && World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameModeAthena>() : nullptr;
	if (GameMode && GameMode->ServerBotManager)
	{
		GameMode->ServerBotManager->PlacePlayerBotsInAircraft(This->Aircrafts);
	}
}

void AFortGameStateAthena::Init()
{
	Memory::HookDetour(ImageBase + 0x11D3C40, CanUpdateGamePhaseStepHook);
	Memory::HookDetour(ImageBase + 0x1925678, PlacePlayersInAircraftHook, &PlacePlayersInAircraftOG);
}

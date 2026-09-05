#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/GameFramework/GameMode.h"

void AFortGameModeAthena::FinishWorldInitializationHook(AFortGameModeAthena* This, AFortWorldManager* WorldManager) {
	AFortGameModeZone::FinishWorldInitializationHook(This, WorldManager);
	FinishWorldInitializationOG(This, WorldManager);
}

bool AFortGameModeAthena::ReadyToStartMatchHook(AFortGameModeAthena* This) {
	if (This->bWorldIsReady
		&& This->MatchState == MatchState::WaitingToStart) {

		return This->CountReadyPlayers() >= This->WarmupRequiredPlayerCount;
	}

	return false;
}

void AFortGameModeAthena::Init() {
	Memory::HookDetour(ImageBase + 0x4551FC0, FinishWorldInitializationHook, &FinishWorldInitializationOG);
	Memory::HookDetour(ImageBase + 0x456AE14, ReadyToStartMatchHook);
}

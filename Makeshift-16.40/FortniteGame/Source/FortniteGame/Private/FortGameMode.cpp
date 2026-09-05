#include "pch.h"

void AFortGameMode::FinishWorldInitializationHook(AFortGameMode* This, AFortWorldManager* WorldManager) {
	FinishWorldInitializationOG(This, WorldManager);
}

void AFortGameMode::Init() {
	Memory::HookDetour(ImageBase + 0x495747C, FinishWorldInitializationHook, &FinishWorldInitializationOG);
}

#include "pch.h"

void AFortGameModeZone::FinishWorldInitializationHook(AFortGameModeZone* This, AFortWorldManager* WorldManager) {
	AFortGameMode::FinishWorldInitializationHook(This, WorldManager);
}

void AFortGameModeZone::Init() {
	Memory::SwapVTableEntry(AFortGameModeZone::GetDefaultObj()->VTable, 0x151, FinishWorldInitializationHook);
}

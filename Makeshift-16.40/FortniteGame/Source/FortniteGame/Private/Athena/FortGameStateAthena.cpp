#include "pch.h"

bool AFortGameStateAthena::CanUpdateGamePhaseStepHook(AFortGameStateAthena* This)
{
	return true;
}

void AFortGameStateAthena::Init()
{
	Memory::HookDetour(ImageBase + 0x11D3C40, CanUpdateGamePhaseStepHook);
}

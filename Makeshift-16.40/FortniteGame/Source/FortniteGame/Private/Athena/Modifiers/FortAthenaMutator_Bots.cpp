#include "pch.h"

void AFortAthenaMutator_Bots::InitializeMMRInfos()
{
	void (*Fn)(AFortAthenaMutator_Bots*) = decltype(Fn)(ImageBase + 0x465DC44);
	Fn(this);
}

void AFortAthenaMutator_Bots::UnpauseWarmupHook(AFortAthenaMutator_Bots* This)
{
	UnpauseWarmupOG(This);

	if (This->CachedGameMode)
	{
		This->CachedGameMode->UnPauseWarmup();
	}
}

void AFortAthenaMutator_Bots::Init()
{
	Memory::HookDetour(ImageBase + 0x465CFA4, UnpauseWarmupHook, &UnpauseWarmupOG);
}

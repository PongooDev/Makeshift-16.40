#include "pch.h"

void AFortPlayerControllerZone::ServerAcknowledgePossession_Implementation(APawn* P) {
	Super::ServerAcknowledgePossession_Implementation(P);

	if (P) {
		AFortPlayerState* FortPlayerState = PlayerState ? PlayerState->Cast<AFortPlayerState>() : nullptr;
		if (FortPlayerState) {
			FortPlayerState->SetHasStartedPlaying(true);
		}
	}
}

void AFortPlayerControllerZone::ServerAcknowledgePossessionHook(AFortPlayerControllerZone* This, APawn* P) {
	This->ServerAcknowledgePossession_Implementation(P);
}

void AFortPlayerControllerZone::Init() {
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerControllerZone>(275, ServerAcknowledgePossessionHook);
}

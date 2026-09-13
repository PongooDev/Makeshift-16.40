#include "pch.h"

void UFortControllerComponent_Aircraft::ServerAttemptAircraftJump_Implementation(const FRotator& ClientRotation) {
	AActor* Owner = GetOwner();
	AController* Controller = Owner ? Owner->Cast<AController>() : nullptr;
	UWorld* World = GetWorld();
	AFortGameStateAthena* GameState = World && World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
	AGameModeBase* GameMode = World ? World->AuthorityGameMode : nullptr;
	if (!Controller || !GameState || !GameMode || !GameState->IsAircraftJumpAllowed(Controller)) {
		return;
	}

	bIsAttemptingAircraftJump = true;
	GameMode->RestartPlayer(Controller);
	bIsAttemptingAircraftJump = false;

	Controller->SetControlRotation(ClientRotation);
	ExitAircraft();
}

void UFortControllerComponent_Aircraft::ServerAttemptAircraftJumpHook(UFortControllerComponent_Aircraft* This, const FRotator& ClientRotation) {
	This->ServerAttemptAircraftJump_Implementation(ClientRotation);
}

void UFortControllerComponent_Aircraft::Init() {
	Memory::SwapVTableEntryInAllSubClasses<UFortControllerComponent_Aircraft>(148, ServerAttemptAircraftJumpHook);
}

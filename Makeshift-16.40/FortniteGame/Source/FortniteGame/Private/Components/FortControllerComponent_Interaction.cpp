#include "pch.h"

void UFortControllerComponent_Interaction::ServerNotifyStartLongUse_Implementation(AActor* ReceivingActor) {
	AActor* Owner = GetOwner();
	AController* Controller = Owner ? Owner->Cast<AController>() : nullptr;
	AFortPlayerPawn* PlayerPawn = Controller && Controller->Pawn ? Controller->Pawn->Cast<AFortPlayerPawn>() : nullptr;
	if (!PlayerPawn || !ReceivingActor) {
		return;
	}

	PlayerPawn->StartedInteractSearch();

	IFortInteractInterface* InteractInterface = static_cast<IFortInteractInterface*>(ReceivingActor->GetInterfaceAddress(IFortInteractInterface::StaticClass()));
	if (InteractInterface) {
		InteractInterface->ServerNotifyStartLongUse(PlayerPawn);
	}
}

void UFortControllerComponent_Interaction::ServerNotifyEndLongUse_Implementation(AActor* ReceivingActor) {
	AActor* Owner = GetOwner();
	AController* Controller = Owner ? Owner->Cast<AController>() : nullptr;
	AFortPlayerPawn* PlayerPawn = Controller && Controller->Pawn ? Controller->Pawn->Cast<AFortPlayerPawn>() : nullptr;
	if (!PlayerPawn || !ReceivingActor) {
		return;
	}

	PlayerPawn->EndedInteractSearch();

	IFortInteractInterface* InteractInterface = static_cast<IFortInteractInterface*>(ReceivingActor->GetInterfaceAddress(IFortInteractInterface::StaticClass()));
	if (InteractInterface) {
		InteractInterface->ServerNotifyEndLongUse(PlayerPawn);
	}
}

void UFortControllerComponent_Interaction::ServerNotifyStartLongUseHook(UFortControllerComponent_Interaction* This, AActor* ReceivingActor) {
	This->ServerNotifyStartLongUse_Implementation(ReceivingActor);
}

void UFortControllerComponent_Interaction::ServerNotifyEndLongUseHook(UFortControllerComponent_Interaction* This, AActor* ReceivingActor) {
	This->ServerNotifyEndLongUse_Implementation(ReceivingActor);
}

void UFortControllerComponent_Interaction::Init() {
	Memory::SwapVTableEntryInAllSubClasses<UFortControllerComponent_Interaction>(146, ServerNotifyStartLongUseHook);
	Memory::SwapVTableEntryInAllSubClasses<UFortControllerComponent_Interaction>(148, ServerNotifyEndLongUseHook);
}

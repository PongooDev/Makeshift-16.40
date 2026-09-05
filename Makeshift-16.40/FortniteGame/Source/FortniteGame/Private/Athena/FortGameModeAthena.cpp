#include "pch.h"
#include "Engine/Source/Runtime/Engine/Classes/GameFramework/GameMode.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/World.h"

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

APawn* AFortGameModeAthena::SpawnDefaultPawnForHook(AFortGameModeAthena* This, AController* NewPlayer, AActor* StartSpot) {
	AFortPlayerControllerAthena* PlayerController = NewPlayer ? NewPlayer->Cast<AFortPlayerControllerAthena>() : nullptr;
	APlayerState* PlayerState = NewPlayer ? NewPlayer->PlayerState : nullptr;
	if (!PlayerController && !(PlayerState && PlayerState->bIsABot)) {
		UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no player controller"));
		return nullptr;
	}

	if (!StartSpot) {
		UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no start spot"));
		return nullptr;
	}

	UClass* PawnClass = This->GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass) {
		UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no default pawn class"));
		return nullptr;
	}

	UWorld* World = This->GetWorld();
	if (!World) {
		UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no world"));
		return nullptr;
	}

	float DefaultHalfHeight = 0.0f;
	if (APawn* DefaultPawn = static_cast<APawn*>(PawnClass->DefaultObject)) {
		DefaultHalfHeight = DefaultPawn->GetDefaultHalfHeight();
	}

	AActor* SpawnSpot = StartSpot;
	APawn* ResultPawn = nullptr;

	for (int32 Attempt = 0; Attempt <= 4 && !ResultPawn; ++Attempt) {
		if (!SpawnSpot) {
			break;
		}

		// Don't allow pawn to be spawned with any pitch or roll
		FRotator StartRotation(ForceInit);
		StartRotation.Yaw = SpawnSpot->K2_GetActorRotation().Yaw;
		FVector StartLocation = SpawnSpot->K2_GetActorLocation();
		StartLocation.Z += DefaultHalfHeight + 24.0f;

		bool bSpawnLocationBlocked = false;

		if (AGameStateBase* GameState = This->GameState) {
			for (APlayerState* OtherPlayerState : GameState->PlayerArray) {
				if (!OtherPlayerState) {
					continue;
				}

				APawn* OtherPawn = OtherPlayerState->PawnPrivate;
				if (!OtherPawn || OtherPawn->Controller == NewPlayer) {
					continue;
				}

				if (FVector::DistSquaredXY(OtherPawn->K2_GetActorLocation(), StartLocation) < 36.0f) {
					bSpawnLocationBlocked = true;
					break;
				}
			}
		}

		if (!bSpawnLocationBlocked) {
			FTransform Transform = FTransform(StartRotation, StartLocation);

			FActorSpawnParameters SpawnInfo;
			SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
			SpawnInfo.bDeferConstruction = true;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ResultPawn = World->SpawnActor<APawn>(PawnClass, Transform, SpawnInfo);
			if (ResultPawn) {
				ResultPawn->FinishSpawning(Transform);
				break;
			}
		}

		SpawnSpot = This->ChoosePlayerStart(NewPlayer);
	}

	if (!ResultPawn) {
		UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Couldn't spawn Pawn of type %hs for %hs"), PawnClass->GetName().c_str(), NewPlayer->GetName().c_str());
	}

	return ResultPawn;
}

void AFortGameModeAthena::InitGameStateHook(AFortGameModeAthena* This) {
	InitGameStateOG(This);

	AFortGameStateAthena* GameState = This->GameState ? This->GameState->Cast<AFortGameStateAthena>() : nullptr;
	if (!GameState) {
		return;
	}

	UFortGameInstance* GameInstance = This->GetWorld()->OwningGameInstance ? This->GetWorld()->OwningGameInstance->Cast<UFortGameInstance>() : nullptr;
	UFortPlaylistAthena* Playlist = (GameInstance && GameInstance->PlaylistManager) ? GameInstance->PlaylistManager->GetPlaylist(This->CurrentPlaylistName) : nullptr;
	if (!Playlist) {
		return;
	}

	This->GameSession->MaxPlayers = Playlist->MaxPlayers;
	This->GameSession->MaxPartySize = Playlist->MaxTeamSize;

	This->MaxPlayerCount = Playlist->MaxPlayers;

	GameState->bStormReachedFinalPosition = false;

	GameState->SetCurrentPlaylistId(Playlist->PlaylistId);

	GameState->CurrentPlaylistInfo.SetBasePlaylist(Playlist);
	GameState->CurrentPlaylistInfo.MarkArrayDirty();

	GameState->OnRep_CurrentPlaylistInfo();
}

void AFortGameModeAthena::Init() {
	Memory::HookDetour(ImageBase + 0x4551FC0, FinishWorldInitializationHook, &FinishWorldInitializationOG);
	Memory::HookDetour(ImageBase + 0x456AE14, ReadyToStartMatchHook);
	Memory::HookDetour(ImageBase + 0x45742D8, SpawnDefaultPawnForHook);
	Memory::HookDetour(ImageBase + 0x455CAD8, InitGameStateHook, &InitGameStateOG);
}

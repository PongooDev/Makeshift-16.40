#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/GameFramework/GameMode.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/World.h"
#include "FortniteGame/Source/FortniteGame/Public/Athena/Modifiers/FortAthenaMutator.h"
#include "FortniteGame/Source/FortniteGame/Public/Analytics/FortAnalytics.h"

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

	This->CreateSpawningPolicyManager();

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

	UFortAthenaLivingWorldManager::HandleCurrentPlaylistLoaded(GameState, This->CurrentPlaylistName);
}

static int16 NextWorldPlayerId = 0;

void AFortGameModeAthena::PostLoginHook(AFortGameModeAthena* This, APlayerController* NewPlayer) {
	PostLoginOG(This, NewPlayer);

	AFortPlayerState* PlayerState = NewPlayer && NewPlayer->PlayerState ? NewPlayer->PlayerState->Cast<AFortPlayerState>() : nullptr;
	if (PlayerState && PlayerState->WorldPlayerId == INDEX_NONE) {
		PlayerState->WorldPlayerId = NextWorldPlayerId++;
	}
}

namespace FortAthenaGameModeCVars
{
	static int32& bMegaStormEnabled = *reinterpret_cast<int32*>(ImageBase + 0x92D4328);
}

void AFortGameModeAthena::SpawnInitialSafeZone() {
	AFortGameStateAthena* FortGameState = GameState ? GameState->Cast<AFortGameStateAthena>() : nullptr;
	AFortAthenaMapInfo* MapInfo = FortGameState ? FortGameState->MapInfo : nullptr;
	if (!FortGameState || !MapInfo) {
		return;
	}

	bSafeZoneActive = true;
	SafeZonePhase = 0;
	SendPreSafeZonePhaseChangedAnalytics(true);
	FortGameState->SetSafeZonePhase(static_cast<uint8>(SafeZonePhase));
	SendPostSafeZonePhaseChangedAnalytics();
	TriggerBuildingGameplayActorSpawning();

	bool bShouldSpawnSafeZoneIndicator = true;
	{
		FMutatorContextIterator MutatorIt(nullptr, this, false);
		for (; MutatorIt.CurrentMutator; MutatorIt.SetCurrentMutator(MutatorIt.CurrentMutatorIndex + 1)) {
			if (MutatorIt.CurrentMutator->FortGameModeAthena_ShouldSpawnSafeZoneIndicator(bShouldSpawnSafeZoneIndicator) == EFortMutatorReturnValue::OverrideReturn) {
				break;
			}
		}
	}

	if (!bShouldSpawnSafeZoneIndicator) {
		UE_LOG(LogFort, Verbose, TEXT("FortGameModeAthena::SpawnInitialSafeZone bShouldSpawnSafeZoneIndicator == false, skipping spawning Safe Zone Indicator"));
		return;
	}

	if (SafeZoneLocations.Num() <= 0) {
		UE_LOG(LogFort, Warning, TEXT("FortGameModeAthena::SpawnInitialSafeZone: SafeZoneLocations is empty, the safe zone indicator cannot be placed"));
		return;
	}

	if (!SafeZoneIndicator) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;

		UClass* IndicatorClass = SafeZoneIndicatorClass;
		if (IndicatorClass && !IndicatorClass->IsChildOf(AFortSafeZoneIndicator::StaticClass())) {
			IndicatorClass = nullptr;
		}

		SafeZoneIndicator = static_cast<AFortSafeZoneIndicator*>(GetWorld()->SpawnActor(IndicatorClass, nullptr, SpawnParams));
	}

	if (!SafeZoneIndicator) {
		return;
	}

	static_cast<FVector&>(SafeZoneIndicator->NextCenter) = SafeZoneLocations[0];
	static_cast<FVector&>(SafeZoneIndicator->LastCenter) = SafeZoneLocations[0];
	FortGameState->SetSafeZoneIndicator(SafeZoneIndicator);
	UpdateSafeZoneEventDriven();

	const FFortSafeZoneDefinition& SafeZoneDefinition = MapInfo->SafeZoneDefinition;
	SafeZoneIndicator->LastRadius = FMath::Max(SafeZoneDefinition.GetRadius(0), MapInfo->SafeZoneStartingRadius.GetValueAtLevel(0.f));

	SafeZoneIndicator->NextRadius = SafeZoneIndicator->LastRadius;
	SafeZoneIndicator->NextMegaStormGridCellThickness = FMath::RoundToInt(SafeZoneDefinition.GetMegaStormGridCellThickness(0));
	if (MegaStormManager) {
		SafeZoneIndicator->MegaStormDelayTimeBeforeDestruction = MegaStormManager->MinDelayTimeBeforeDestruction;
	}

	UpdateNextNextSafeZone(FortGameState, MapInfo);

	if (!SafeZoneIndicator->bSafezoneEventDriven) {
		SafeZoneIndicator->SetShrinkDuration(SafeZoneDefinition.GetShrinkTime(SafeZonePhase));
		SafeZoneIndicator->OffsetShrinkTimesFromServerTime(SafeZoneDefinition.GetWaitTime(SafeZonePhase));
	}

	SafeZoneIndicator->SetSafeZoneRadiusAndCenter(SafeZoneIndicator->LastRadius, SafeZoneIndicator->LastCenter);
	SafeZoneInsideChecks();
}

void AFortGameModeAthena::StartNewSafeZonePhase(int32 NewSafeZonePhase) {
	AFortGameStateAthena* FortGameState = GameState ? GameState->Cast<AFortGameStateAthena>() : nullptr;
	AFortAthenaMapInfo* MapInfo = FortGameState ? FortGameState->MapInfo : nullptr;
	if (!FortGameState || !MapInfo || !SafeZoneIndicator) {
		return;
	}

	SendPreSafeZonePhaseChangedAnalytics(false);
	if (NewSafeZonePhase >= 0) {
		SafeZonePhase = NewSafeZonePhase;
	} else {
		++SafeZonePhase;
	}

	FortGameState->SetSafeZonePhase(static_cast<uint8>(SafeZonePhase));
	SendPostSafeZonePhaseChangedAnalytics();
	TriggerBuildingGameplayActorSpawning();

	if (SafeZonePhase < 0 || SafeZonePhase >= SafeZoneLocations.Num()) {
		UE_LOG(LogFort, Warning, TEXT("FortGameModeAthena::StartNewSafeZonePhase: SafeZonePhase %d has no safe zone location (%d locations)"), SafeZonePhase, SafeZoneLocations.Num());
		return;
	}

	const FFortSafeZoneDefinition& SafeZoneDefinition = MapInfo->SafeZoneDefinition;
	if (NewSafeZonePhase != -1) {
		const int32 NextSafeZoneIndex = SafeZonePhase + 1 < SafeZoneLocations.Num() ? SafeZonePhase + 1 : SafeZonePhase;
		static_cast<FVector&>(SafeZoneIndicator->LastCenter) = SafeZoneLocations[SafeZonePhase];
		static_cast<FVector&>(SafeZoneIndicator->NextCenter) = SafeZoneLocations[NextSafeZoneIndex];
		SafeZoneIndicator->LastRadius = SafeZoneDefinition.GetRadius(SafeZonePhase);
		SafeZoneIndicator->NextRadius = SafeZonePhase + 1 >= SafeZoneLocations.Num() ? SafeZoneIndicator->LastRadius : SafeZoneDefinition.GetRadius(SafeZonePhase + 1);
	} else {
		static_cast<FVector&>(SafeZoneIndicator->LastCenter) = SafeZoneIndicator->NextCenter;
		static_cast<FVector&>(SafeZoneIndicator->NextCenter) = SafeZoneLocations[SafeZonePhase];
		SafeZoneIndicator->LastRadius = SafeZoneIndicator->Radius;
		SafeZoneIndicator->NextRadius = SafeZoneDefinition.GetRadius(SafeZonePhase);
	}

	SafeZoneIndicator->NextMegaStormGridCellThickness = FMath::RoundToInt(SafeZoneDefinition.GetMegaStormGridCellThickness(SafeZonePhase));
	UpdateNextNextSafeZone(FortGameState, MapInfo);
	UpdateSafeZoneEventDriven();

	if (!SafeZoneIndicator->bSafezoneEventDriven) {
		SafeZoneIndicator->SetShrinkDuration(SafeZoneDefinition.GetShrinkTime(SafeZonePhase));
		SafeZoneIndicator->OffsetShrinkTimesFromServerTime(SafeZoneDefinition.GetWaitTime(SafeZonePhase));
	}

	const int32 NumBuildingGridCellsThick = SafeZoneIndicator->NextMegaStormGridCellThickness;
	if (NumBuildingGridCellsThick < 1) {
		UE_LOG(LogFort, Log, TEXT("FortGameModeAthena: No MegaStorm on SafeZone[%d].  GridCellThickness is less than 1.0."), SafeZonePhase);
	} else if (FortAthenaGameModeCVars::bMegaStormEnabled) {
		if (MegaStormManager) {
			const FVector Center = SafeZoneIndicator->GetSafeZoneCenter();
			const float Radius = SafeZoneIndicator->Radius;
			UE_LOG(LogFort, Log, TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."), SafeZonePhase, NumBuildingGridCellsThick);
			for (int32 GridCellThickness = NumBuildingGridCellsThick; GridCellThickness >= 0; --GridCellThickness) {
				MegaStormManager->StartMegaStormCircle(Center, Radius, GridCellThickness);
			}
		} else {
			UE_LOG(LogFort, Log, TEXT("FortGameModeAthena: No MegaStorm on SafeZone[%d].  MegaStormManager is invalid"), SafeZonePhase);
		}
	} else {
		UE_LOG(LogFort, Log, TEXT("FortGameModeAthena: No MegaStorm on SafeZone[%d].  FortAthenaGameModeCVars::bMegaStormEnabled is disabled."), SafeZonePhase);
	}

	SafeZoneIndicator->ForceNetUpdate();

	TArray<AActor*> PlayerPawns;
	UGameplayStatics::GetAllActorsOfClass(this, AFortPlayerPawnAthena::StaticClass(), &PlayerPawns);
	for (int32 Index = 0; Index < PlayerPawns.Num(); ++Index) {
		AFortPlayerPawnAthena* PlayerPawn = static_cast<AFortPlayerPawnAthena*>(PlayerPawns[Index]);
		if (PlayerPawn && PlayerPawn->AbilitySystemComponent) {
			PlayerPawn->AbilitySystemComponent->SetActiveGameplayEffectLevel(PlayerPawn->GE_OutsideSafeZone_Handle, SafeZonePhase);
		}
	}

	if (NewSafeZonePhase != -1) {
		FortGameState->RefreshSafeZone(SafeZoneIndicator->NextCenter, SafeZoneIndicator->NextRadius);
	}

	if (SkyTubeDisablePhase.AsInteger(0.f) > 0 && SafeZonePhase == SkyTubeDisablePhase.AsInteger(0.f)) {
		FortGameState->ShutDownSkyTubes();
	}

	if (OnSafeZoneUpdated.InvocationList.Num() > 0) {
		OnSafeZoneUpdated.ProcessMulticastDelegate(nullptr);
	}

	const FAthenaSafeZoneTelemetryInfo PreviousSafeZone(SafeZoneIndicator->LastCenter, SafeZoneIndicator->LastRadius, static_cast<uint8>(SafeZonePhase - 1), EFortSafeZoneState::Holding);
	const FAthenaSafeZoneTelemetryInfo NewSafeZone(SafeZoneIndicator->NextCenter, SafeZoneIndicator->NextRadius, static_cast<uint8>(SafeZonePhase), EFortSafeZoneState::Starting);
	UFortAnalytics::FireEvent_AthenaNewSafeZone(this, NewSafeZone, PreviousSafeZone);

	for (int32 Index = 0; Index < AlivePlayers.Num(); ++Index) {
		AFortPlayerControllerAthena* PlayerController = AlivePlayers[Index];
		AFortPlayerStateAthena* PlayerStateAthena = PlayerController && PlayerController->PlayerState ? PlayerController->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
		if (!PlayerStateAthena || !PlayerStateAthena->DeathInfo.bInitialized || !PlayerStateAthena->RespawnData.bRespawnDataAvailable || PlayerStateAthena->IsReadyToRespawn() || !IsRespawningAllowed(PlayerStateAthena)) {
			continue;
		}

		FVector RespawnLocation;
		FRotator RespawnRotation;
		float RespawnCameraDistance = 0.f;
		if (CalculateRespawnData(RespawnLocation, RespawnRotation, RespawnCameraDistance, PlayerController, PlayerStateAthena, nullptr)) {
			PlayerStateAthena->SetRespawnData(RespawnLocation, RespawnRotation, RespawnCameraDistance);
			PlayerController->PrepareClientForRespawning();
		}
	}
}

void AFortGameModeAthena::SpawnInitialSafeZoneHook(AFortGameModeAthena* This) {
	This->SpawnInitialSafeZone();
}

void AFortGameModeAthena::StartNewSafeZonePhaseHook(AFortGameModeAthena* This, int32 NewSafeZonePhase) {
	This->StartNewSafeZonePhase(NewSafeZonePhase);
}

void AFortGameModeAthena::PauseWarmup() {
	bWarmupPaused = true;
}

void AFortGameModeAthena::UnPauseWarmup() {
	bWarmupPaused = false;
}

void AFortGameModeAthena::CreateSpawningPolicyManager() {
	if (SpawningPolicyManager) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FTransform Transform{};
	Transform.Rotation = FQuat(0.f, 0.f, 0.f, 1.f);
	Transform.Translation = FVector(0.f, 0.f, 0.f);
	Transform.Scale3D = FVector(1.f, 1.f, 1.f);
	SpawningPolicyManager = GetWorld()->SpawnActor<AFortAthenaSpawningPolicyManager>(AFortAthenaSpawningPolicyManager::StaticClass(), Transform, SpawnParams);
	if (!SpawningPolicyManager) {
		UE_LOG(LogFort, Warning, TEXT("AFortGameModeAthena::CreateSpawningPolicyManager : Failed to spawn the spawning policy manager"));
		return;
	}

	UE_LOG(LogFort, Log, TEXT("AFortGameModeAthena::CreateSpawningPolicyManager : Created %hs"), SpawningPolicyManager->GetName().c_str());
}

void AFortGameModeAthena::CreateServerBotManager() {
	if (ServerBotManager) {
		return;
	}

	if (!GetPlaylistEnableBots()) {
		UE_LOG(LogFort, Log, TEXT("AFortGameModeAthena::CreateServerBotManager : Bots are not enabled for this playlist, not creating the server bot manager"));
		return;
	}

	if (!ServerBotManagerClass) {
		ServerBotManagerClass = UFortServerBotManagerAthena::StaticClass();
		UE_LOG(LogFort, Warning, TEXT("AFortGameModeAthena::CreateServerBotManager : ServerBotManagerClass is not set, falling back to %hs"), ServerBotManagerClass->GetName().c_str());
	}

	ServerBotManager = NewObject<UFortServerBotManagerAthena>(this, ServerBotManagerClass);
	if (!ServerBotManager) {
		UE_LOG(LogFort, Warning, TEXT("AFortGameModeAthena::CreateServerBotManager : Failed to create the server bot manager from %hs"), ServerBotManagerClass->GetName().c_str());
		return;
	}

	UWorld* World = GetWorld();
	ServerBotManager->CachedGameMode = this;
	ServerBotManager->CachedGameState = GameState ? GameState->Cast<AFortGameStateAthena>() : nullptr;
	ServerBotManager->CachedAIPopulationTracker = UAthenaAISystem::GetAIPopulationTracker(World);

	UAthenaAISystem* AthenaAISystem = (World && World->AISystem) ? World->AISystem->Cast<UAthenaAISystem>() : nullptr;
	if (AthenaAISystem) {
		AthenaAISystem->PlayerBotManager = ServerBotManager;
	}

	AFortGameplayMutator* Mutator = GetMutatorByClass(this, AFortAthenaMutator_Bots::StaticClass());
	AFortAthenaMutator_Bots* BotMutator = Mutator ? Mutator->Cast<AFortAthenaMutator_Bots>() : nullptr;
	if (BotMutator && !ServerBotManager->CachedBotMutator) {
		ServerBotManager->SetBotMutator(BotMutator);
		ServerBotManager->bBotHostileToHumanPlayersOnly = BotMutator->bBotHostileToHumanPlayersOnly;
	}

	UE_LOG(LogFort, Log, TEXT("AFortGameModeAthena::CreateServerBotManager : Created %hs"), ServerBotManager->GetName().c_str());
}

void AFortGameModeAthena::OnPlaylistDataLoadedHook(AFortGameModeAthena* This) {
	OnPlaylistDataLoadedOG(This);

	AFortGameStateAthena* FortGameState = This->GameState ? This->GameState->Cast<AFortGameStateAthena>() : nullptr;
	if (!FortGameState || !FortGameState->GetCurrentPlaylistData()) {
		return;
	}

	This->CreateServerBotManager();
}

void AFortGameModeAthena::Init() {
	Memory::HookDetour(ImageBase + 0x4551FC0, FinishWorldInitializationHook, &FinishWorldInitializationOG);
	Memory::HookDetour(ImageBase + 0x456AE14, ReadyToStartMatchHook);
	Memory::HookDetour(ImageBase + 0x45742D8, SpawnDefaultPawnForHook);
	Memory::HookDetour(ImageBase + 0x455CAD8, InitGameStateHook, &InitGameStateOG);
	Memory::HookDetour(ImageBase + 0x4563F08, OnPlaylistDataLoadedHook, &OnPlaylistDataLoadedOG);
	Memory::HookDetour(ImageBase + 0x4566D60, PostLoginHook, &PostLoginOG);
	Memory::HookDetour(ImageBase + 0x4574D64, SpawnInitialSafeZoneHook);
	Memory::HookDetour(ImageBase + 0x45799A4, StartNewSafeZonePhaseHook);
}

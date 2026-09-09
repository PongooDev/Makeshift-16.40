#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/FortGlobals.h"

static UFortWeaponItemDefinition* GetFinishingWeaponFromDamageCauser(AActor* DamageCauser) {
	if (!DamageCauser) {
		return nullptr;
	}

	if (AFortWeapon* Weapon = DamageCauser->Cast<AFortWeapon>()) {
		return Weapon->WeaponData;
	}

	if (ABuildingTrap* Trap = DamageCauser->Cast<ABuildingTrap>()) {
		return Trap->TrapData ? Trap->TrapData->Cast<UFortWeaponItemDefinition>() : nullptr;
	}

	if (AFortProjectileBase* Projectile = DamageCauser->Cast<AFortProjectileBase>()) {
		AActor* ProjectileOwner = Projectile->Owner;
		if (AFortWeapon* OwnerWeapon = ProjectileOwner ? ProjectileOwner->Cast<AFortWeapon>() : nullptr) {
			return OwnerWeapon->WeaponData;
		}

		if (ABuildingTrap* OwnerTrap = ProjectileOwner ? ProjectileOwner->Cast<ABuildingTrap>() : nullptr) {
			return OwnerTrap->TrapData ? OwnerTrap->TrapData->Cast<UFortWeaponItemDefinition>() : nullptr;
		}
	}

	return nullptr;
}

static void GetOwnedGameplayTagsFromAbilitySystem(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer& OutTags) {
	const IGameplayTagAssetInterface* TagInterface = AbilitySystemComponent ? static_cast<const IGameplayTagAssetInterface*>(AbilitySystemComponent->GetInterfaceAddress(IGameplayTagAssetInterface::StaticClass())) : nullptr;
	if (TagInterface) {
		TagInterface->GetOwnedGameplayTags(OutTags);
	}
}

void AFortPlayerPawnAthena::InitializeDeathInfoOnPawnDeath(const FGameplayTagContainer& InTags, AController* EventInstigator, AController* DBNOFinisher, const AActor* DamageCauser) {
	AFortPlayerStateAthena* PlayerStateAthena = PlayerState ? PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
	if (!PlayerStateAthena) {
		return;
	}

	UWorld* World = GetWorld();
	AFortGameModeAthena* GameMode = World && World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameModeAthena>() : nullptr;

	AController* FinisherController = DBNOFinisher ? DBNOFinisher : EventInstigator;
	AFortPlayerStateAthena* FinisherPS = FinisherController && FinisherController->PlayerState ? FinisherController->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
	AFortPawn* FinisherPawn = FinisherController && FinisherController->Pawn ? FinisherController->Pawn->Cast<AFortPawn>() : nullptr;

	AController* DownerController = DBNOData.EventInstigator.Get();
	AFortPlayerStateAthena* DownerPS = DownerController && DownerController->PlayerState ? DownerController->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;

	FDeathInfo DeathInfo{};
	DeathInfo.bDBNO = false;
	DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(InTags, bWasDBNOOnDeath);
	DeathInfo.DeathClassSlot = 0xFF;
	DeathInfo.FinisherOrDowner = FinisherPS ? static_cast<AActor*>(FinisherPS) : (FinisherPawn ? static_cast<AActor*>(FinisherPawn) : static_cast<AActor*>(PlayerStateAthena));
	DeathInfo.Downer = bWasDBNOOnDeath && DownerPS ? static_cast<AActor*>(DownerPS) : DeathInfo.FinisherOrDowner;
	DeathInfo.Distance = 0.0f;
	if (FinisherPS && FinisherPS != PlayerStateAthena && FinisherPawn) {
		DeathInfo.Distance = FinisherPawn->GetDistanceTo(this);
	}

	if (InTags.HasTag(UFortGlobals::GameplayTags().DamageFalling)) {
		DeathInfo.Distance = LastFallDistance;
	}

	DeathInfo.DeathLocation = K2_GetActorLocation();
	DeathInfo.bInitialized = false;
	DeathInfo.DeathTags = InTags;
	if (FinisherPawn) {
		GetOwnedGameplayTagsFromAbilitySystem(FinisherPawn->AbilitySystemComponent, DeathInfo.FinisherOrDownerTags);
	}

	GetOwnedGameplayTagsFromAbilitySystem(AbilitySystemComponent, DeathInfo.VictimTags);
	DeathInfo.bRespawnEnabledOnDeath = GameMode ? GameMode->IsRespawningAllowed(PlayerStateAthena) : false;

	PlayerStateAthena->InitializeDeathInfo(DeathInfo);
}

bool AFortPlayerPawnAthena::IsTeamOutOfGame(AFortPlayerStateAthena& KilledPS, AFortPlayerControllerAthena* FPCA) const {
	if (FPCA) {
		return FPCA->IsTeamOutOfGame();
	}

	return false;
}

void AFortPlayerPawnAthena::SetMatchPlacement(int32 Placement, AFortPlayerStateAthena& KilledPS, AFortPlayerControllerAthena* FPCA) const {
	if (FPCA) {
		FPCA->SetMatchPlacement(Placement);
	}
}

void AFortPlayerPawnAthena::HandleDeath(float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser) {
	AFortPlayerControllerAthena* FPCA = Controller ? Controller->Cast<AFortPlayerControllerAthena>() : nullptr;
	AFortPlayerStateAthena* KilledPS = PlayerState ? PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;

	UWorld* World = GetWorld();
	AFortGameModeAthena* GameMode = World && World->AuthorityGameMode ? World->AuthorityGameMode->Cast<AFortGameModeAthena>() : nullptr;
	AFortGameStateAthena* GameState = GameMode && GameMode->GameState ? GameMode->GameState->Cast<AFortGameStateAthena>() : nullptr;

	AController* DBNOFinisher = DBNOData.DBNOFinisher.Get();
	AController* KillerController = DBNOFinisher ? DBNOFinisher : EventInstigator;
	AFortPlayerStateAthena* KillerPS = KillerController && KillerController->PlayerState ? KillerController->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
	APawn* KillerPawn = KillerController ? KillerController->Pawn : nullptr;

	if (Role == ENetRole::ROLE_Authority && KilledPS) {
		InitializeDeathInfoOnPawnDeath(InTags, EventInstigator, DBNOFinisher, DamageCauser);

		if (KillerPS && KillerPS != KilledPS && KillerPS->TeamIndex != KilledPS->TeamIndex) {
			KillerPS->SetKillScore(KillerPS->KillScore + 1);
			KillerPS->ClientReportKill(KilledPS);

			const int32 NewTeamKillScore = KillerPS->TeamKillScore + 1;
			const TArray<TWeakObjectPtr<AFortPlayerStateAthena>>& TeamMembers = KillerPS->GetTeamMembers();
			bool bKillerReported = false;
			for (int32 Index = 0; Index < TeamMembers.Num(); ++Index) {
				AFortPlayerStateAthena* TeamMember = TeamMembers[Index].Get();
				if (!TeamMember) {
					continue;
				}

				TeamMember->SetTeamKillScore(NewTeamKillScore);
				TeamMember->ClientReportTeamKill(NewTeamKillScore);
				if (TeamMember == KillerPS) {
					bKillerReported = true;
				}
			}

			if (!bKillerReported) {
				KillerPS->SetTeamKillScore(NewTeamKillScore);
				KillerPS->ClientReportTeamKill(NewTeamKillScore);
			}
		}

		if (GameMode) {
			GameMode->Killed(KillerController, Controller, this, InTags);
		}
	}

	HandleDeathOG(this, Damage, InTags, EffectContext, EventInstigator, DamageCauser);

	if (Role == ENetRole::ROLE_Authority && GameMode && GameState && KilledPS && FPCA) {
		const bool bRespawnAllowed = GameMode->IsRespawningAllowed(KilledPS) && !InTags.HasTag(UFortGlobals::GameplayTags().PlayerDisconnected);
		if (!bRespawnAllowed) {
			if (IsTeamOutOfGame(*KilledPS, FPCA)) {
				SetMatchPlacement(GameState->TeamsLeft, *KilledPS, FPCA);
			}

			GameMode->RemoveFromAlivePlayers(FPCA, KillerPS, KillerPawn, GetFinishingWeaponFromDamageCauser(DamageCauser), KilledPS->DeathInfo.DeathCause, false);
		}
	}
}

void AFortPlayerPawnAthena::HandleDeathHook(AFortPlayerPawnAthena* This, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser) {
	This->HandleDeath(Damage, InTags, EffectContext, EventInstigator, DamageCauser);
}

void AFortPlayerPawnAthena::Init() {
	HandleDeathOG = decltype(HandleDeathOG)(ImageBase + 0x45F1638);
	Memory::SwapVTableEntryInAllSubClasses<AFortPlayerPawnAthena>(345, HandleDeathHook);
}

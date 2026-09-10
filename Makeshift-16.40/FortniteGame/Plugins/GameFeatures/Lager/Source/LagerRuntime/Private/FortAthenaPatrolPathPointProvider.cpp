#include "pch.h"

static void (*InitPointProviderOG)(AFortAthenaPatrolPathPointProvider* This, AFortAthenaPatrolPath* PatrolPath, bool bUsePatrolPathTags) = nullptr;

bool AFortAthenaPatrolPathPointProvider::IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const
{
	const AFortAthenaPatrolPath* PatrolPath = RuntimePatrolPathWeakPtr.Get();
	if (!CachedGameState || !PatrolPath)
	{
		return false;
	}

	if (PatrolPath->MaxConcurrentUsage >= 0 && PatrolPath->CurrentConcurrentUsage >= PatrolPath->MaxConcurrentUsage)
	{
		return false;
	}

	const FVector ProviderLocation = GetActorLocation();
	if (!IFortAthenaLivingWorldPointProviderInterface::IsPointWithinSafeZoneRules(PointFilter, ProviderLocation, CachedGameState, PointProviderMaxRadius))
	{
		return false;
	}

	if (PointFilter.bCheckForActorDensity)
	{
		const UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(const_cast<AFortAthenaPatrolPathPointProvider*>(this));
		const float ActorDensityComputationRadius = LivingWorldManager ? FMath::Max(0.f, LivingWorldManager->GetDensityComputationDistance() - PointProviderMaxRadius) : 0.f;
		if (!IFortAthenaLivingWorldPointProviderInterface::IsPointWithinDensityRules(PointFilter, ProviderLocation, LivingWorldManager, ActorDensityComputationRadius))
		{
			return false;
		}
	}

	return true;
}

bool AFortAthenaPatrolPathPointProvider::GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, const float ActorDensityComputationRadius, FVector& OutPosition, FRotator& OutRotation) const
{
	if (!CachedGameState)
	{
		return false;
	}

	const AActor* LocationSource = this;
	if (bSpawnOnPatrolPath)
	{
		const AFortAthenaPatrolPath* PatrolPath = RuntimePatrolPathWeakPtr.Get();
		if (!PatrolPath || PatrolPath->PatrolPoints.Num() <= 0)
		{
			return false;
		}

		const int32 PatrolPointIndex = PatrolPath->bUseRandomStartupPatrolPoint ? FMath::RandHelper(PatrolPath->PatrolPoints.Num()) : 0;
		const AFortAthenaPatrolPoint* PatrolPoint = PatrolPath->PatrolPoints[PatrolPointIndex];
		if (!PatrolPoint)
		{
			return false;
		}
		LocationSource = PatrolPoint;
	}

	const FVector Location = LocationSource->GetActorLocation();
	if (!IFortAthenaLivingWorldPointProviderInterface::IsPointWithinSafeZoneRules(PointFilter, Location, CachedGameState, 0.f))
	{
		return false;
	}

	if (PointFilter.bCheckForActorDensity)
	{
		const UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(const_cast<AFortAthenaPatrolPathPointProvider*>(this));
		if (!IFortAthenaLivingWorldPointProviderInterface::IsPointWithinDensityRules(PointFilter, Location, LivingWorldManager, ActorDensityComputationRadius))
		{
			return false;
		}
	}

	OutPosition = Location;
	OutRotation = const_cast<AActor*>(LocationSource)->K2_GetActorRotation();
	return true;
}

void AFortAthenaPatrolPathPointProvider::InitPointProviderHook(AFortAthenaPatrolPathPointProvider* This, AFortAthenaPatrolPath* PatrolPath, bool bUsePatrolPathTags)
{
	InitPointProviderOG(This, PatrolPath, bUsePatrolPathTags);

	if (!This->CachedGameState)
	{
		UWorld* World = This->GetWorld();
		This->CachedGameState = World && World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
	}

	if (UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(This))
	{
		LivingWorldManager->RegisterPointProvider(This);
	}
}

bool AFortAthenaPatrolPathPointProvider::GetValidLocationHook(const IFortAthenaLivingWorldPointProviderInterface* This, const FFortAthenaLivingWorldPointProviderFilterRules* PointFilter, float ActorDensityComputationRadius, FVector* OutPosition, FRotator* OutRotation)
{
	const AFortAthenaPatrolPathPointProvider* PointProvider = reinterpret_cast<const AFortAthenaPatrolPathPointProvider*>(reinterpret_cast<const uint8*>(This) - 0x220);
	return PointProvider->GetValidLocation(*PointFilter, ActorDensityComputationRadius, *OutPosition, *OutRotation);
}

bool AFortAthenaPatrolPathPointProvider::IsEnabledHook(const IFortAthenaLivingWorldPointProviderInterface* This, const FFortAthenaLivingWorldPointProviderFilterRules* PointFilter)
{
	const AFortAthenaPatrolPathPointProvider* PointProvider = reinterpret_cast<const AFortAthenaPatrolPathPointProvider*>(reinterpret_cast<const uint8*>(This) - 0x220);
	return PointProvider->IsEnabled(*PointFilter);
}

void AFortAthenaPatrolPathPointProvider::Init()
{
	Memory::HookDetour(ImageBase + 0x3C7B28C, InitPointProviderHook, &InitPointProviderOG);
	Memory::SwapVTableEntryInAllSubClasses<AFortAthenaPatrolPathPointProvider>(2, GetValidLocationHook, 0x220);
	Memory::SwapVTableEntryInAllSubClasses<AFortAthenaPatrolPathPointProvider>(7, IsEnabledHook, 0x220);
}

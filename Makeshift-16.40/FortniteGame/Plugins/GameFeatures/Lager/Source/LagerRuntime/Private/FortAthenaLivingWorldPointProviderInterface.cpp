#include "pch.h"

bool IFortAthenaLivingWorldPointProviderInterface::IsPointWithinSafeZoneRules(const FFortAthenaLivingWorldPointProviderFilterRules& FilterRules, const FVector& TargetPoint, const AFortGameStateAthena* GameState, float RadiusSlack)
{
	if (!GameState)
	{
		return false;
	}

	const IFortSafeZoneInterface* SafeZoneInterface = GameState->GetSafeZoneInterface();
	if (!SafeZoneInterface->GetSafeZoneState())
	{
		return true;
	}

	if (FilterRules.bOnlyPointsInSafeZone)
	{
		const FVector SafeZoneCenter = SafeZoneInterface->GetSafeZoneCenter();
		const float SafeZoneRadius = SafeZoneInterface->GetSafeZoneRadius();
		const float Distance = FMath::Max(0.f, sqrtf(FVector::DistSquaredXY(TargetPoint, SafeZoneCenter)) - RadiusSlack);
		return Distance <= SafeZoneRadius;
	}

	return true;
}

bool IFortAthenaLivingWorldPointProviderInterface::IsPointWithinDensityRules(const FFortAthenaLivingWorldPointProviderFilterRules& FilterRules, const FVector& TargetPoint, const UFortAthenaLivingWorldManager* LivingWorldManager, float ActorDensityComputationRadius)
{
	if (!FilterRules.bCheckForActorDensity)
	{
		return true;
	}

	if (!LivingWorldManager)
	{
		return false;
	}

	return LivingWorldManager->GetActorDensity(TargetPoint, ActorDensityComputationRadius) < LivingWorldManager->GetMaxActorDensity();
}

bool IFortAthenaLivingWorldPointProviderInterface::IsPointWithinFilterRules(const FFortAthenaLivingWorldPointProviderFilterRules& FilterRules, const FVector& TargetPoint, const AFortGameStateAthena* GameState, const UFortAthenaLivingWorldManager* LivingWorldManager, float ActorDensityComputationRadius, float RadiusSlack)
{
	return IsPointWithinDensityRules(FilterRules, TargetPoint, LivingWorldManager, ActorDensityComputationRadius)
		&& IsPointWithinSafeZoneRules(FilterRules, TargetPoint, GameState, RadiusSlack);
}

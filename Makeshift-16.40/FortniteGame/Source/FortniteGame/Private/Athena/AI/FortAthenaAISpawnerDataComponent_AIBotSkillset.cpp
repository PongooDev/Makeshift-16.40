#include "pch.h"

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestHealingItems(const TArray<FFortBotHealingItems>& SkillSetItems, TArray<FFortBotDigestedHealingItems>& DigestedItems, float Level)
{
	DigestedItems.Empty(SkillSetItems.Num());

	for (int32 Index = 0; Index < SkillSetItems.Num(); ++Index)
	{
		const FFortBotHealingItems& Item = SkillSetItems[Index];

		FFortBotDigestedHealingItems DigestedItem{};
		DigestedItem.UseItemResourceThreshold = Item.UseItemResourceThreshold.GetValueAtLevel(Level);
		DigestedItem.ItemTags = Item.ItemTags;

		DigestedItems.Add(DigestedItem);
	}
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestRangedWeaponSkill(const FRangedWeaponSkill& WeaponSkill, FDigestedRangedWeaponSkill& Digested, float Level)
{
	Digested.DelayBetweenShots = WeaponSkill.DelayBetweenShots.GetValueAtLevel(Level);
	Digested.DelayDeviationTimeBetweenShots = WeaponSkill.DelayDeviationTimeBetweenShots.GetValueAtLevel(Level);
	Digested.TriggerHoldDuration = WeaponSkill.TriggerHoldDuration.GetValueAtLevel(Level);
	Digested.TriggerHoldDeviationTime = WeaponSkill.TriggerHoldDeviationTime.GetValueAtLevel(Level);
	Digested.DelayBeforeFirstShot = WeaponSkill.DelayBeforeFirstShot.GetValueAtLevel(Level);
	Digested.ShotDelayAfterTargeting = WeaponSkill.ShotDelayAfterTargeting.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestAimingSkillSet(UFortAthenaAIBotAimingSkillSet* SkillSet, UFortAthenaAIBotAimingDigestedSkillSet* Digested, float Level)
{
	Digested->bAllowScanAroundWhileSwimming = SkillSet->AllowScanAroundWhileSwimming.GetValueAtLevel(Level) > 0.f;
	Digested->TrackingReactionTime = SkillSet->TrackingReactionTime.GetValueAtLevel(Level);
	Digested->TrackingInterpTime = SkillSet->TrackingInterpTime.GetValueAtLevel(Level);
	Digested->MaxTrackingPredictionError = SkillSet->MaxTrackingPredictionError.GetValueAtLevel(Level);
	Digested->MaxTrackingOffsetErrorMultiplier = SkillSet->MaxTrackingOffsetErrorMultiplier.GetValueAtLevel(Level);
	Digested->AdjustedTrackingOffsetErrorMultiplierAgainstAIs = SkillSet->AdjustedTrackingOffsetErrorMultiplierAgainstAIs.GetValueAtLevel(Level);
	Digested->TrackingErrorUpdateInterval = SkillSet->TrackingErrorUpdateInterval.GetValueAtLevel(Level);
	Digested->TrackingInAirVelocityThreshold = SkillSet->TrackingInAirVelocityThreshold.GetValueAtLevel(Level);
	Digested->TrackingInAirHeightDeltaThreshold = SkillSet->TrackingInAirHeightDeltaThreshold.GetValueAtLevel(Level);
	Digested->TargetAcquisitionRate = SkillSet->TargetAcquisitionRate.GetValueAtLevel(Level);
	Digested->MaxDistanceEvaluationErrorRatio = SkillSet->MaxDistanceEvaluationErrorRatio.GetValueAtLevel(Level);
	Digested->TargetingUpdateInterval = SkillSet->TargetingUpdateInterval.GetValueAtLevel(Level);
	Digested->TargetingUpdateIntervalMaxDeviation = SkillSet->TargetingUpdateIntervalMaxDeviation.GetValueAtLevel(Level);
	Digested->ReachLeashLimitToleranceDistance = SkillSet->ReachLeashLimitToleranceDistance.GetValueAtLevel(Level);
	Digested->bShootFloorTrapOnlyWhenHigherThanTrap = SkillSet->ShootFloorTrapOnlyWhenHigherThanTrap.GetValueAtLevel(Level) > 0.f;
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestAttackingSkillSet(UFortAthenaAIBotAttackingSkillSet* SkillSet, UFortAthenaAIBotAttackingDigestedSkillSet* Digested, float Level)
{
	const float MaxDistanceToEngageMelee = SkillSet->MaxDistanceToEngageMelee.GetValueAtLevel(Level);
	Digested->MaxDistanceToEngageMeleeSq = MaxDistanceToEngageMelee * MaxDistanceToEngageMelee;
	Digested->bOnlyEngageMeleeAgainstThreatThatHasNoRangeWeapon = SkillSet->OnlyEngageMeleeAgainstThreatThatHasNoRangeWeapon.GetValueAtLevel(Level) > 0.f;
	const float MaxDistanceToConsiderAsAnAlternateTarget = SkillSet->MaxDistanceToConsiderAsAnAlternateTarget.GetValueAtLevel(Level);
	Digested->MaxDistanceToConsiderAsAnAlternateTargetSq = MaxDistanceToConsiderAsAnAlternateTarget * MaxDistanceToConsiderAsAnAlternateTarget;
	Digested->bEnableAlternateTargetRequiredTags = SkillSet->EnableAlternateTargetRequiredTags.GetValueAtLevel(Level) > 0.f;
	Digested->bEnableWTFBehavior = SkillSet->EnableWTFBehavior.GetValueAtLevel(Level) > 0.f;
	Digested->MinCooldownDelayBetweenMeleeAttackAttempts = SkillSet->MinCooldownDelayBetweenMeleeAttackAttempts.GetValueAtLevel(Level);
	Digested->MaxCooldownDelayBetweenMeleeAttackAttempts = SkillSet->MaxCooldownDelayBetweenMeleeAttackAttempts.GetValueAtLevel(Level);
	Digested->MinThrowableCount = SkillSet->MinThrowableCount.AsInteger(Level);
	Digested->MaxThrowableCount = SkillSet->MaxThrowableCount.AsInteger(Level);
	Digested->ThrowableCooldownMin = SkillSet->ThrowableCooldownMin.GetValueAtLevel(Level);
	Digested->ThrowableCooldownMax = SkillSet->ThrowableCooldownMax.GetValueAtLevel(Level);
	const float ThrowableMinimumRange = SkillSet->ThrowableMinimumRange.GetValueAtLevel(Level);
	Digested->ThrowableMinimumRangeSquared = ThrowableMinimumRange * ThrowableMinimumRange;
	Digested->bThrowableEvaluatorActive = SkillSet->ThrowableEvaluatorActive.GetValueAtLevel(Level) > 0.f;
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestBuildingSkillSet(UFortAthenaAIBotBuildingSkillSet* SkillSet, UFortAthenaAIBotBuildingDigestedSkillSet* Digested, float Level)
{
	Digested->DefensiveBuildingDelayBetweenBuilds = SkillSet->DefensiveBuildingDelayBetweenBuilds.GetValueAtLevel(Level);
	Digested->DefensiveBuildingDelayDeviationBetweenBuilds = SkillSet->DefensiveBuildingDelayDeviationBetweenBuilds.GetValueAtLevel(Level);
	Digested->DelayBetweenBuildPieces = SkillSet->DelayBetweenBuildPieces.GetValueAtLevel(Level);
	Digested->ForceEquipBuildToolDuration = SkillSet->ForceEquipBuildToolDuration.GetValueAtLevel(Level);
	Digested->StealWallTurboBuildDetectionTime = SkillSet->StealWallTurboBuildDetectionTime.GetValueAtLevel(Level);
	Digested->StealWallAfterNumberOfTurboBuiltWall = SkillSet->StealWallAfterNumberOfTurboBuiltWall.AsInteger(Level);
	Digested->StealWallEfficiency = SkillSet->StealWallEfficiency.GetValueAtLevel(Level);
	for (int32 Index = 0; Index < 5; ++Index)
	{
		Digested->StealWallBuildingTemplateWeights[Index] = SkillSet->StealWallBuildingTemplateWeights[Index].GetValueAtLevel(Level);
	}
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestDBNOSkillSet(UFortAthenaAIBotDBNOSkillSet* SkillSet, UFortAthenaAIBotDBNODigestedSkillSet* Digested, float Level)
{
	Digested->MaxDBNOCrawlingResponseTime = SkillSet->MaxDBNOCrawlingResponseTime.GetValueAtLevel(Level);
	Digested->MaxDBNOCrawlingResponseTimeDeviation = SkillSet->MaxDBNOCrawlingResponseTimeDeviation.GetValueAtLevel(Level);
	Digested->AllyEvaluationTime = SkillSet->AllyEvaluationTime.GetValueAtLevel(Level);
	Digested->AllyEvaluationTimeDeviation = SkillSet->AllyEvaluationTimeDeviation.GetValueAtLevel(Level);
	Digested->AllyEvaluationMaxDistance = SkillSet->AllyEvaluationMaxDistance.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestEmoteSkillSet(UFortAthenaAIBotEmoteSkillSet* SkillSet, UFortAthenaAIBotEmoteDigestedSkillSet* Digested, float Level)
{
	Digested->InfiniteEmoteMinDuration = SkillSet->InfiniteEmoteMinDuration.GetValueAtLevel(Level);
	Digested->InfiniteEmoteMaxDuration = SkillSet->InfiniteEmoteMaxDuration.GetValueAtLevel(Level);
	const float DanceOnKillMaxDistanceFromKill = SkillSet->DanceOnKillMaxDistanceFromKill.GetValueAtLevel(Level);
	Digested->DanceOnKillMaxDistanceFromKillSqr = DanceOnKillMaxDistanceFromKill * DanceOnKillMaxDistanceFromKill;
	Digested->DanceOnKillMaxTimeFromKill = SkillSet->DanceOnKillMaxTimeFromKill.GetValueAtLevel(Level);
	Digested->DanceOnKillMinTimeFromLastTry = SkillSet->DanceOnKillMinTimeFromLastTry.GetValueAtLevel(Level);
	Digested->DanceOnKillChanceToDanceOnBots = SkillSet->DanceOnKillChanceToDanceOnBots.GetValueAtLevel(Level);
	Digested->DanceOnKillChanceToDanceOnPlayers = SkillSet->DanceOnKillChanceToDanceOnPlayers.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestEvasiveManeuversSkillSet(UFortAthenaAIBotEvasiveManeuversSkillSet* SkillSet, UFortAthenaAIBotEvasiveManeuversDigestedSkillSet* Digested, float Level)
{
	Digested->JumpDelay = SkillSet->JumpDelay.GetValueAtLevel(Level);
	Digested->JumpRandomDeviationDelay = SkillSet->JumpRandomDeviationDelay.GetValueAtLevel(Level);
	Digested->CrouchDelay = SkillSet->CrouchDelay.GetValueAtLevel(Level);
	Digested->CrouchRandomDeviationDelay = SkillSet->CrouchRandomDeviationDelay.GetValueAtLevel(Level);
	Digested->JetpackStrafeDelay = SkillSet->JetpackStrafeDelay.GetValueAtLevel(Level);
	Digested->JetpackStrafeRandomDeviationDelay = SkillSet->JetpackStrafeRandomDeviationDelay.GetValueAtLevel(Level);
	Digested->DodgeDelay = SkillSet->DodgeDelay.GetValueAtLevel(Level);
	Digested->DodgeRandomDeviationDelay = SkillSet->DodgeRandomDeviationDelay.GetValueAtLevel(Level);
	Digested->CrouchOverlayWeight = SkillSet->CrouchOverlayWeight.GetValueAtLevel(Level);
	Digested->JumpOverlayWeight = SkillSet->JumpOverlayWeight.GetValueAtLevel(Level);
	Digested->JetpackStrafeOverlayWeight = SkillSet->JetpackStrafeOverlayWeight.GetValueAtLevel(Level);
	Digested->NoOverlayWeight = SkillSet->NoOverlayWeight.GetValueAtLevel(Level);
	Digested->DodgeWeight = SkillSet->DodgeWeight.GetValueAtLevel(Level);
	Digested->DodgeDistanceMax = SkillSet->DodgeDistanceMax.GetValueAtLevel(Level);
	Digested->DodgeDistanceMin = SkillSet->DodgeDistanceMin.GetValueAtLevel(Level);
	Digested->JetpackStrafeDistanceMax = SkillSet->JetpackStrafeDistanceMax.GetValueAtLevel(Level);
	Digested->JetpackStrafeDistanceMin = SkillSet->JetpackStrafeDistanceMin.GetValueAtLevel(Level);
	Digested->JetpackStrafeActivationTime = SkillSet->JetpackStrafeActivationTime.GetValueAtLevel(Level);
	Digested->JetpackStrafeActivationTimeRandomDeviation = SkillSet->JetpackStrafeActivationTimeRandomDeviation.GetValueAtLevel(Level);
	Digested->JetpackStrafeTime = SkillSet->JetpackStrafeTime.GetValueAtLevel(Level);
	Digested->JetpackStrafeTimeRandomDeviation = SkillSet->JetpackStrafeTimeRandomDeviation.GetValueAtLevel(Level);
	Digested->CrouchTimeMax = SkillSet->CrouchTimeMax.GetValueAtLevel(Level);
	Digested->CrouchTimeMin = SkillSet->CrouchTimeMin.GetValueAtLevel(Level);
	const float DodgeMaxDistance = SkillSet->DodgeMaxDistance.GetValueAtLevel(Level);
	Digested->DodgeMaxDistanceSquared = DodgeMaxDistance * DodgeMaxDistance;
	const float CrouchMaxDistance = SkillSet->CrouchMaxDistance.GetValueAtLevel(Level);
	Digested->CrouchMaxDistanceSquared = CrouchMaxDistance * CrouchMaxDistance;
	const float JumpMaxDistance = SkillSet->JumpMaxDistance.GetValueAtLevel(Level);
	Digested->JumpMaxDistanceSquared = JumpMaxDistance * JumpMaxDistance;
	const float JetpackStrafeMaxDistance = SkillSet->JetpackStrafeMaxDistance.GetValueAtLevel(Level);
	Digested->JetpackStrafeMaxDistanceSquared = JetpackStrafeMaxDistance * JetpackStrafeMaxDistance;
	const float AvoidProjectilesReactionDistanceMax = SkillSet->AvoidProjectilesReactionDistanceMax.GetValueAtLevel(Level);
	Digested->AvoidProjectilesReactionDistanceSqr = AvoidProjectilesReactionDistanceMax * AvoidProjectilesReactionDistanceMax;
	Digested->AvoidProjectilesReactionTimeMin = SkillSet->AvoidProjectilesReactionTimeMin.GetValueAtLevel(Level);
	Digested->AvoidProjectilesReactionTimeMax = SkillSet->AvoidProjectilesReactionTimeMax.GetValueAtLevel(Level);
	Digested->AvoidProjectilesEvasiveDistanceMin = SkillSet->AvoidProjectilesEvasiveDistanceMin.GetValueAtLevel(Level);
	Digested->AvoidProjectilesEvasiveDistanceMax = SkillSet->AvoidProjectilesEvasiveDistanceMax.GetValueAtLevel(Level);
	Digested->AvoidPhysicsObjectsReactionDistanceMin = SkillSet->AvoidPhysicsObjectsReactionDistanceMin.GetValueAtLevel(Level);
	Digested->AvoidPhysicsObjectsReactionDistanceMax = SkillSet->AvoidPhysicsObjectsReactionDistanceMax.GetValueAtLevel(Level);
	Digested->AvoidPhysicsObjectsSpeedMin = SkillSet->AvoidPhysicsObjectsSpeedMin.GetValueAtLevel(Level);
	Digested->AvoidPhysicsObjectsSpeedMax = SkillSet->AvoidPhysicsObjectsSpeedMax.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestHarvestSkillSet(UFortAthenaAIBotHarvestSkillSet* SkillSet, UFortAthenaAIBotHarvestDigestedSkillSet* Digested, float Level)
{
	Digested->DelayBetweenHarvest = SkillSet->DelayBetweenHarvest.GetValueAtLevel(Level);
	Digested->DeviationTimeBetweenHarvest = SkillSet->DeviationTimeBetweenHarvest.GetValueAtLevel(Level);
	const float HarvestingMaxDistance = SkillSet->HarvestingMaxDistance.GetValueAtLevel(Level);
	Digested->HarvestingMaxDistanceSquared = HarvestingMaxDistance * HarvestingMaxDistance;
	Digested->WeakSpotHitProbability = SkillSet->WeakSpotHitProbability.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestHealingSkillSet(UFortAthenaAIBotHealingSkillSet* SkillSet, UFortAthenaAIBotHealingDigestedSkillSet* Digested, float Level)
{
	DigestHealingItems(SkillSet->HealthItems, Digested->HealthItems, Level);
	DigestHealingItems(SkillSet->ShieldItems, Digested->ShieldItems, Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestInventorySkillSet(UFortAthenaAIBotInventorySkillSet* SkillSet, UFortAthenaAIBotInventoryDigestedSkillSet* Digested, float Level)
{
	Digested->DefaultWeaponSelectionDistance = SkillSet->DefaultWeaponSelectionDistance.GetValueAtLevel(Level);
	Digested->DefaultWeaponSelectionDistanceDeviation = SkillSet->DefaultWeaponSelectionDistanceDeviation.GetValueAtLevel(Level);
	Digested->bHasInfiniteResources = SkillSet->HasInfiniteResources.GetValueAtLevel(Level) > 0.f;
	Digested->NoWeaponGiveWeaponAfterTime = SkillSet->NoWeaponGiveWeaponAfterTime.GetValueAtLevel(Level);
	Digested->NoWeaponNoPlayerConeDistance = SkillSet->NoWeaponNoPlayerConeDistance.GetValueAtLevel(Level);
	Digested->NoWeaponNoPlayerConeFOV = SkillSet->NoWeaponNoPlayerConeFOV.GetValueAtLevel(Level);
	Digested->NoWeaponLootTierGroup = SkillSet->NoWeaponLootTierGroup;
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestLootingSkillSet(UFortAthenaAIBotLootingSkillSet* SkillSet, UFortAthenaAIBotLootingDigestedSkillSet* Digested, float Level)
{
	Digested->ThresholdDistanceToSwitchLootItem = SkillSet->ThresholdDistanceToSwitchLootItem.GetValueAtLevel(Level);
	const float ThresholdDistanceToRescanForBetterLoot = SkillSet->ThresholdDistanceToRescanForBetterLoot.GetValueAtLevel(Level);
	Digested->ThresholdDistanceSquaredToRescanForBetterLoot = ThresholdDistanceToRescanForBetterLoot * ThresholdDistanceToRescanForBetterLoot;
	Digested->ThresholdTimeToRescanForBetterLoot = SkillSet->ThresholdTimeToRescanForBetterLoot.GetValueAtLevel(Level);
	const float LootStateEvaluationRadius = SkillSet->LootStateEvaluationRadius.GetValueAtLevel(Level);
	Digested->LootStateEvaluationRadiusSq = LootStateEvaluationRadius * LootStateEvaluationRadius;
	Digested->MinLootDurationPerPOI = SkillSet->MinLootDurationPerPOI.GetValueAtLevel(Level);
	Digested->MaxLootDurationPerPOI = SkillSet->MaxLootDurationPerPOI.GetValueAtLevel(Level);
	Digested->LootPickupInteractionTime = SkillSet->LootPickupInteractionTime.GetValueAtLevel(Level);
	Digested->LootPickupInteractionDeviationTime = SkillSet->LootPickupInteractionDeviationTime.GetValueAtLevel(Level);
	Digested->Distance2DScore = SkillSet->Distance2DScore.GetValueAtLevel(Level);
	Digested->HeightScore = SkillSet->HeightScore.GetValueAtLevel(Level);
	Digested->ThreatMaxScore = SkillSet->ThreatMaxScore.GetValueAtLevel(Level);
	Digested->PrioritizeWeaponScore = SkillSet->PrioritizeWeaponScore.GetValueAtLevel(Level);
	Digested->PoiSelectionDistanceScore = SkillSet->PoiSelectionDistanceScore.GetValueAtLevel(Level);
	Digested->PoiSelectionBotPresenceScore = SkillSet->PoiSelectionBotPresenceScore.GetValueAtLevel(Level);
	Digested->OctreeBoxHalfExtent.X = SkillSet->OctreeBoxExtent.X * 0.5f;
	Digested->OctreeBoxHalfExtent.Y = SkillSet->OctreeBoxExtent.Y * 0.5f;
	Digested->OctreeBoxHalfExtent.Z = SkillSet->OctreeBoxExtent.Z * 0.5f;
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestMovementSkillSet(UFortAthenaAIBotMovementSkillSet* SkillSet, UFortAthenaAIBotMovementDigestedSkillSet* Digested, float Level)
{
	Digested->SlowDownDistance = SkillSet->SlowDownDistance.GetValueAtLevel(Level);
	Digested->TraversalSpeedEstimation = SkillSet->TraversalSpeedEstimation.GetValueAtLevel(Level);
	Digested->TraversalSpeedEstimationWithThreat = SkillSet->TraversalSpeedEstimationWithThreat.GetValueAtLevel(Level);
	Digested->GliderDeployMinAngle = SkillSet->GliderDeployMinAngle.GetValueAtLevel(Level);
	Digested->GliderDeployMaxAngle = SkillSet->GliderDeployMaxAngle.GetValueAtLevel(Level);
	Digested->GliderNoiseMaxDistance = SkillSet->GliderNoiseMaxDistance.GetValueAtLevel(Level);
	Digested->GliderNoiseMinDelay = SkillSet->GliderNoiseMinDelay.GetValueAtLevel(Level);
	Digested->GliderNoiseMaxDelay = SkillSet->GliderNoiseMaxDelay.GetValueAtLevel(Level);
	Digested->JumpOffMinAngle = SkillSet->JumpOffMinAngle.GetValueAtLevel(Level);
	Digested->JumpOffMaxAngle = SkillSet->JumpOffMaxAngle.GetValueAtLevel(Level);
	Digested->MaxPatrolDistance = SkillSet->MaxPatrolDistance.GetValueAtLevel(Level);
	Digested->MaxPatrolDistanceRandomDeviation = SkillSet->MaxPatrolDistanceRandomDeviation.GetValueAtLevel(Level);
	Digested->WobbleProbability = SkillSet->WobblingProbability.GetValueAtLevel(Level);
	Digested->MaxDelayBetweenWobblingMovement = SkillSet->MaxDelayBetweenWobblingMovement.GetValueAtLevel(Level);
	Digested->MaxDelayBetweenWobblingMovementRandomDeviation = SkillSet->MaxDelayBetweenWobblingMovementRandomDeviation.GetValueAtLevel(Level);
	Digested->bAllowSwimWobble = SkillSet->AllowSwimWobble.GetValueAtLevel(Level) > 0.f;
	Digested->MaxWobblingDuration = SkillSet->MaxWobblingDuration.GetValueAtLevel(Level);
	Digested->MaxWobblingDurationRandomDeviation = SkillSet->MaxWobblingDurationRandomDeviation.GetValueAtLevel(Level);
	Digested->MaxWobblingIntensity = SkillSet->MaxWobblingIntensity.GetValueAtLevel(Level);
	Digested->WobblingIntensityDeviation = SkillSet->WobblingIntensityDeviation.GetValueAtLevel(Level);
	Digested->MaxWobblingFrequency = SkillSet->MaxWobblingFrequency.GetValueAtLevel(Level);
	Digested->WobblingFrequencyDeviation = SkillSet->WobblingFrequencyDeviation.GetValueAtLevel(Level);
	Digested->WobblingStickToPathCorridorStrength = SkillSet->WobblingStickToPathCorridorStrength.GetValueAtLevel(Level);
	Digested->MaxAfterLaunchedPauseTime = SkillSet->MaxAfterLaunchedPauseTime.GetValueAtLevel(Level);
	Digested->AfterLaunchedPauseTimeDeviation = SkillSet->AfterLaunchedPauseTimeDeviation.GetValueAtLevel(Level);
	Digested->bSteerMovementWhenLaunched = SkillSet->SteerMovementWhenLaunched.GetValueAtLevel(Level) > 0.f;
	Digested->SteerMovementWhenLaunchedDirectionUpdateTime = SkillSet->SteerMovementWhenLaunchedDirectionUpdateTime.GetValueAtLevel(Level);
	Digested->MaxReactionTimeToDangerZone = SkillSet->MaxReactionTimeToDangerZone.GetValueAtLevel(Level);
	Digested->MaxReactionTimeToDangerZoneDeviation = SkillSet->MaxReactionTimeToDangerZoneDeviation.GetValueAtLevel(Level);
	Digested->bLimitBlockingObstacleAngle = SkillSet->LimitBlockingObstacleAngle.GetValueAtLevel(Level) > 0.f;
	Digested->SwimSprintJumpDelay = SkillSet->SwimSprintJumpDelay.GetValueAtLevel(Level);
	Digested->SwimSprintJumpDelayDeviation = SkillSet->SwimSprintJumpDelayDeviation.GetValueAtLevel(Level);
	Digested->SwimUnblockJumpHeightThreshold = SkillSet->SwimUnblockJumpHeightThreshold.GetValueAtLevel(Level);
	Digested->bSwimSprintJumpNav2D = SkillSet->SwimSprintJumpNav2D.GetValueAtLevel(Level) > 0.f;
	Digested->MoveToRangeAttackMinOffset = SkillSet->MoveToRangeAttackMinOffset.GetValueAtLevel(Level);
	Digested->MoveToRangeAttackMaxOffset = SkillSet->MoveToRangeAttackMaxOffset.GetValueAtLevel(Level);
	Digested->LKPMinOffset = SkillSet->LKPMinOffset.GetValueAtLevel(Level);
	Digested->LKPMaxOffset = SkillSet->LKPMaxOffset.GetValueAtLevel(Level);
	Digested->SandTunnelJumpMinTime = SkillSet->SandTunnelJumpMinTime.GetValueAtLevel(Level);
	Digested->SandTunnelJumpMaxTime = SkillSet->SandTunnelJumpMaxTime.GetValueAtLevel(Level);
	Digested->SandTunnelBurrowedAndHiddenChance = SkillSet->SandTunnelBurrowedAndHiddenChance.GetValueAtLevel(Level);
	Digested->SandTunnelBurrowedAndHiddenMinTime = SkillSet->SandTunnelBurrowedAndHiddenMinTime.GetValueAtLevel(Level);
	Digested->SandTunnelBurrowedAndHiddenMaxTime = SkillSet->SandTunnelBurrowedAndHiddenMaxTime.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestPerceptionSkillSet(UFortAthenaAIBotPerceptionSkillSet* SkillSet, UFortAthenaAIBotPerceptionDigestedSkillSet* Digested, float Level)
{
	Digested->SightReactionTime = SkillSet->SightReactionTime.GetValueAtLevel(Level);
	Digested->SightRandomDeviation = SkillSet->SightRandomDeviation.GetValueAtLevel(Level);
	Digested->LoseSightTime = SkillSet->LoseSightTime.GetValueAtLevel(Level);
	Digested->LoseSightRandomDeviation = SkillSet->LoseSightRandomDeviation.GetValueAtLevel(Level);
	Digested->SightSuspicionTime = SkillSet->SightSuspicionTime.GetValueAtLevel(Level);
	Digested->SightSuspicionRandomDeviation = SkillSet->SightSuspicionRandomDeviation.GetValueAtLevel(Level);
	Digested->EnemyMarkedReactionTime = SkillSet->EnemyMarkedReactionTime.GetValueAtLevel(Level);
	Digested->EnemyMarkedReactionRandomDeviation = SkillSet->EnemyMarkedReactionRandomDeviation.GetValueAtLevel(Level);
	Digested->ChancesToHelpOnMarkedEnemy = SkillSet->ChancesToHelpOnMarkedEnemy.GetValueAtLevel(Level);
	Digested->DamageReactionTime = SkillSet->DamageReactionTime.GetValueAtLevel(Level);
	Digested->DamageRandomDeviation = SkillSet->DamageRandomDeviation.GetValueAtLevel(Level);
	Digested->HearingReactionTime = SkillSet->HearingReactionTime.GetValueAtLevel(Level);
	Digested->HearingRandomDeviation = SkillSet->HearingRandomDeviation.GetValueAtLevel(Level);
	Digested->MaxHearingLocationError = SkillSet->MaxHearingLocationError.GetValueAtLevel(Level);
	Digested->ThreatDamageWeight = SkillSet->ThreatDamageWeight.GetValueAtLevel(Level);
	Digested->ThreatDamageWeightMultiplier = SkillSet->ThreatDamageWeightMultiplier.GetValueAtLevel(Level);
	const float ObstacleDistanceOverrideTargeting = SkillSet->ObstacleDistanceOverrideTargeting.GetValueAtLevel(Level);
	Digested->ObstacleDistanceOverrideTargetingSq = ObstacleDistanceOverrideTargeting * ObstacleDistanceOverrideTargeting;
	const float ObstacleForgetDistance = SkillSet->ObstacleForgetDistance.GetValueAtLevel(Level);
	Digested->ObstacleForgetDistanceSq = ObstacleForgetDistance * ObstacleForgetDistance;
	Digested->DBNOWeightModifier = SkillSet->DBNOWeightModifier.GetValueAtLevel(Level);
	Digested->AlertedAccumulatedLoudnessLimit = SkillSet->AlertedAccumulatedLoudnessLimit.GetValueAtLevel(Level);
	Digested->LKPAccumulatedLoudnessLimit = SkillSet->LKPAccumulatedLoudnessLimit.GetValueAtLevel(Level);
	Digested->EnemyMarkingDelay = SkillSet->EnemyMarkingDelay.GetValueAtLevel(Level);
	Digested->EnemyMarkingDelayRandomDeviation = SkillSet->EnemyMarkingDelayRandomDeviation.GetValueAtLevel(Level);
	Digested->AdditionalMarkedEnemyLKPForgetTime = SkillSet->AdditionalMarkedEnemyLKPForgetTime.GetValueAtLevel(Level);
	Digested->AdditionalMarkedEnemyLKPForgetDistance = SkillSet->AdditionalMarkedEnemyLKPForgetDistance.GetValueAtLevel(Level);
	Digested->ProjectileThreatForgetTime = SkillSet->ProjectileThreatForgetTime.GetValueAtLevel(Level);
	Digested->bStealthMeterEnable = SkillSet->StealthMeterEnable.GetValueAtLevel(Level) > 0.f;
	Digested->StealthMeterThreshold = SkillSet->StealthMeterThreshold.GetValueAtLevel(Level);
	Digested->StealthMeterIncreaseSpeed = SkillSet->StealthMeterIncreaseSpeed;
	Digested->StealthMeterDecreaseSpeed = SkillSet->StealthMeterDecreaseSpeed.GetValueAtLevel(Level);
	Digested->bStealthMeterAllowSharedTarget = SkillSet->StealthMeterAllowSharedTarget.GetValueAtLevel(Level) > 0.f;
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestPlayStyleSkillSet(UFortAthenaAIBotPlayStyleSkillSet* SkillSet, UFortAthenaAIBotPlayStyleDigestedSkillSet* Digested, float Level)
{
	Digested->AggressiveTowardsThreatWeight = SkillSet->AggressiveTowardsThreatWeight;
	Digested->DefensiveTowardsThreatWeight = SkillSet->DefensiveTowardsThreatWeight;
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestPropagateAwarenessSkillSet(UFortAthenaAIBotPropagateAwarenessSkillSet* SkillSet, UFortAthenaAIBotPropagateAwarenessDigestedSkillSet* Digested, float Level)
{
	const float PropagationMaxDistance = SkillSet->PropagationMaxDistance.GetValueAtLevel(Level);
	Digested->PropagationMaxDistanceSQ = PropagationMaxDistance * PropagationMaxDistance;
	Digested->CosineFOV = FMath::Cos(FMath::DegreesToRadians(SkillSet->FOV.GetValueAtLevel(Level) * 0.5f));
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestRangeAttackSkillSet(UFortAthenaAIBotRangeAttackSkillSet* SkillSet, UFortAthenaAIBotRangeAttackDigestedSkillSet* Digested, float Level)
{
	Digested->RangedWeaponCategorySkills.Empty(SkillSet->RangedWeaponSkills.Num());

	for (int32 CategoryIndex = 0; CategoryIndex < SkillSet->RangedWeaponSkills.Num(); ++CategoryIndex)
	{
		const FRangedWeaponSkillCategory& Category = SkillSet->RangedWeaponSkills[CategoryIndex];

		FDigestedRangedWeaponSkillCategory DigestedCategory{};
		DigestedCategory.Tags = Category.Tags;
		DigestRangedWeaponSkill(Category.WeaponSkill, DigestedCategory.WeaponSkill, Level);

		for (int32 SpecializationIndex = 0; SpecializationIndex < Category.Specializations.Num(); ++SpecializationIndex)
		{
			const FRangedWeaponSkillCategorySpecialization& Specialization = Category.Specializations[SpecializationIndex];

			FDigestedRangedWeaponSkillCategorySpecialization DigestedSpecialization{};
			DigestedSpecialization.Tags = Specialization.Tags;
			DigestRangedWeaponSkill(Specialization.WeaponSkill, DigestedSpecialization.WeaponSkill, Level);

			DigestedCategory.Specializations.Add(DigestedSpecialization);
		}

		Digested->RangedWeaponCategorySkills.Add(DigestedCategory);
	}
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestReviveSkillSet(UFortAthenaAIBotReviveSkillSet* SkillSet, UFortAthenaAIBotReviveDigestedSkillSet* Digested, float Level)
{
	Digested->AllyEvaluationTime = SkillSet->AllyEvaluationTime.GetValueAtLevel(Level);
	Digested->AllyEvaluationTimeDeviation = SkillSet->AllyEvaluationTimeDeviation.GetValueAtLevel(Level);
	Digested->CooldownOnCancel = SkillSet->CooldownOnCancel.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestUnstuckSkillSet(UFortAthenaAIBotUnstuckSkillSet* SkillSet, UFortAthenaAIBotUnstuckDigestedSkillSet* Digested, float Level)
{
	Digested->bCanTeleportWhenStuck = SkillSet->CanTeleportWhenStuck.GetValueAtLevel(Level) > 0.f;
	Digested->bCanTeleportWhenStuckWithPlayerAround = SkillSet->CanTeleportWhenStuckWithPlayerAround.GetValueAtLevel(Level) > 0.f;
	const float MaxDistanceToPlayerToTeleport = SkillSet->MaxDistanceToPlayerToTeleport.GetValueAtLevel(Level);
	Digested->MaxDistanceSqToPlayerToTeleport = MaxDistanceToPlayerToTeleport * MaxDistanceToPlayerToTeleport;
	Digested->PlayerToPhoebeAngleVisibilityConeToTeleport = SkillSet->PlayerToPhoebeAngleVisibilityConeToTeleport.GetValueAtLevel(Level);
	Digested->TimeBetweenPartialPathToConsiderPathStuck = SkillSet->TimeBetweenPartialPathToConsiderPathStuck.GetValueAtLevel(Level);
	Digested->ConsecutivePartialPathCountToConsiderPathStuck = SkillSet->ConsecutivePartialPathCountToConsiderPathStuck.AsInteger(Level);
	const float DistanceBetweenBlockedPathToConsiderPathStuck = SkillSet->DistanceBetweenBlockedPathToConsiderPathStuck.GetValueAtLevel(Level);
	Digested->DistanceSqBetweenBlockedPathToConsiderPathStuck = DistanceBetweenBlockedPathToConsiderPathStuck * DistanceBetweenBlockedPathToConsiderPathStuck;
	Digested->DistanceBetweenSampleToConsiderPathStuckInWater = SkillSet->DistanceBetweenSampleToConsiderPathStuckInWater.GetValueAtLevel(Level);
	Digested->TimeBetweenSampleToConsiderPathStuckInWater = SkillSet->TimeBetweenSampleToConsiderPathStuckInWater.GetValueAtLevel(Level);
	Digested->DistanceBetweenSampleToConsiderPathStuckOnGround = SkillSet->DistanceBetweenSampleToConsiderPathStuckOnGround.GetValueAtLevel(Level);
	Digested->TimeBetweenSampleToConsiderPathStuckOnGround = SkillSet->TimeBetweenSampleToConsiderPathStuckOnGround.GetValueAtLevel(Level);
	Digested->ConsecutiveBlockedPathCountToConsiderPathStuck = SkillSet->ConsecutiveBlockedPathCountToConsiderPathStuck.AsInteger(Level);
	Digested->MaxSafeZoneIndexToAllowTeleport = SkillSet->MaxSafeZoneIndexToAllowTeleport.AsInteger(Level);
	Digested->TimeToBreakBlockingDoor = SkillSet->TimeToBreakBlockingDoor.GetValueAtLevel(Level);
	Digested->TimeToCloseBlockingDoor = SkillSet->TimeToCloseBlockingDoor.GetValueAtLevel(Level);
	Digested->RiverbedObstacleCollisionNormalThreshold = SkillSet->RiverbedObstacleCollisionNormalThreshold.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::DigestWarmupSkillSet(UFortAthenaAIBotWarmupSkillSet* SkillSet, UFortAthenaAIBotWarmupDigestedSkillSet* Digested, float Level)
{
	Digested->WarmupPlayEmoteBehaviorWeight = SkillSet->WarmupPlayEmoteBehaviorWeight.GetValueAtLevel(Level);
	Digested->WarmupLootAndShootBehaviorWeight = SkillSet->WarmupLootAndShootBehaviorWeight.GetValueAtLevel(Level);
	Digested->WarmupIdleBehaviorWeight = SkillSet->WarmupIdleBehaviorWeight.GetValueAtLevel(Level);
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::GetAIRuntimeParametersClassesHook(void* Provider, TArray<TSubclassOf<UFortAthenaAIRuntimeParameters>>& OutClasses)
{
	UFortAthenaAISpawnerDataComponent_AIBotSkillset* This = reinterpret_cast<UFortAthenaAISpawnerDataComponent_AIBotSkillset*>(reinterpret_cast<uint8*>(Provider) - offsetof(UFortAthenaAISpawnerDataComponent, Pad_28));

	if (This->AimingSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotAimingDigestedSkillSet::StaticClass());
	}

	if (This->AttackingSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotAttackingDigestedSkillSet::StaticClass());
	}

	if (This->BuildingSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotBuildingDigestedSkillSet::StaticClass());
	}

	if (This->DBNOSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotDBNODigestedSkillSet::StaticClass());
	}

	if (This->EmoteSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotEmoteDigestedSkillSet::StaticClass());
	}

	if (This->EvasiveManeuversSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotEvasiveManeuversDigestedSkillSet::StaticClass());
	}

	if (This->HarvestSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotHarvestDigestedSkillSet::StaticClass());
	}

	if (This->HealingSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotHealingDigestedSkillSet::StaticClass());
	}

	if (This->InventorySkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotInventoryDigestedSkillSet::StaticClass());
	}

	if (This->LootingSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotLootingDigestedSkillSet::StaticClass());
	}

	if (This->MovementSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotMovementDigestedSkillSet::StaticClass());
	}

	if (This->PerceptionSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotPerceptionDigestedSkillSet::StaticClass());
	}

	if (This->PlayStyleSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotPlayStyleDigestedSkillSet::StaticClass());
	}

	if (This->PropagateAwarenessSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotPropagateAwarenessDigestedSkillSet::StaticClass());
	}

	if (This->RangeAttackSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotRangeAttackDigestedSkillSet::StaticClass());
	}

	if (This->ReviveSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotReviveDigestedSkillSet::StaticClass());
	}

	if (This->UnstuckSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotUnstuckDigestedSkillSet::StaticClass());
	}

	UFortAthenaAISpawnerDataComponent_PlayerBotSkillset* PlayerBotThis = This->Cast<UFortAthenaAISpawnerDataComponent_PlayerBotSkillset>();
	if (PlayerBotThis && PlayerBotThis->WarmUpSkillSet.Get())
	{
		OutClasses.Add(UFortAthenaAIBotWarmupDigestedSkillSet::StaticClass());
	}

}

uint32 UFortAthenaAISpawnerDataComponent_AIBotSkillset::ExtractHook(void* Provider, UFortAthenaAIRuntimeParameters* RuntimeParameters, int32 Level, uint32 CRC)
{
	UFortAthenaAISpawnerDataComponent_AIBotSkillset* This = reinterpret_cast<UFortAthenaAISpawnerDataComponent_AIBotSkillset*>(reinterpret_cast<uint8*>(Provider) - offsetof(UFortAthenaAISpawnerDataComponent, Pad_28));
	const float SkillLevel = static_cast<float>(Level);
	if (RuntimeParameters == nullptr)
	{
		return CRC;
	}

	UFortAthenaAIBotAimingDigestedSkillSet* AimingDigested = RuntimeParameters->Cast<UFortAthenaAIBotAimingDigestedSkillSet>();
	if (AimingDigested)
	{
		UClass* SkillSetClass = This->AimingSkillSet.Get();
		UFortAthenaAIBotAimingSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotAimingSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestAimingSkillSet(SkillSet, AimingDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotAttackingDigestedSkillSet* AttackingDigested = RuntimeParameters->Cast<UFortAthenaAIBotAttackingDigestedSkillSet>();
	if (AttackingDigested)
	{
		UClass* SkillSetClass = This->AttackingSkillSet.Get();
		UFortAthenaAIBotAttackingSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotAttackingSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestAttackingSkillSet(SkillSet, AttackingDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotBuildingDigestedSkillSet* BuildingDigested = RuntimeParameters->Cast<UFortAthenaAIBotBuildingDigestedSkillSet>();
	if (BuildingDigested)
	{
		UClass* SkillSetClass = This->BuildingSkillSet.Get();
		UFortAthenaAIBotBuildingSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotBuildingSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestBuildingSkillSet(SkillSet, BuildingDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotDBNODigestedSkillSet* DBNODigested = RuntimeParameters->Cast<UFortAthenaAIBotDBNODigestedSkillSet>();
	if (DBNODigested)
	{
		UClass* SkillSetClass = This->DBNOSkillSet.Get();
		UFortAthenaAIBotDBNOSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotDBNOSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestDBNOSkillSet(SkillSet, DBNODigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotEmoteDigestedSkillSet* EmoteDigested = RuntimeParameters->Cast<UFortAthenaAIBotEmoteDigestedSkillSet>();
	if (EmoteDigested)
	{
		UClass* SkillSetClass = This->EmoteSkillSet.Get();
		UFortAthenaAIBotEmoteSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotEmoteSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestEmoteSkillSet(SkillSet, EmoteDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotEvasiveManeuversDigestedSkillSet* EvasiveManeuversDigested = RuntimeParameters->Cast<UFortAthenaAIBotEvasiveManeuversDigestedSkillSet>();
	if (EvasiveManeuversDigested)
	{
		UClass* SkillSetClass = This->EvasiveManeuversSkillSet.Get();
		UFortAthenaAIBotEvasiveManeuversSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotEvasiveManeuversSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestEvasiveManeuversSkillSet(SkillSet, EvasiveManeuversDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotHarvestDigestedSkillSet* HarvestDigested = RuntimeParameters->Cast<UFortAthenaAIBotHarvestDigestedSkillSet>();
	if (HarvestDigested)
	{
		UClass* SkillSetClass = This->HarvestSkillSet.Get();
		UFortAthenaAIBotHarvestSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotHarvestSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestHarvestSkillSet(SkillSet, HarvestDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotHealingDigestedSkillSet* HealingDigested = RuntimeParameters->Cast<UFortAthenaAIBotHealingDigestedSkillSet>();
	if (HealingDigested)
	{
		UClass* SkillSetClass = This->HealingSkillSet.Get();
		UFortAthenaAIBotHealingSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotHealingSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestHealingSkillSet(SkillSet, HealingDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotInventoryDigestedSkillSet* InventoryDigested = RuntimeParameters->Cast<UFortAthenaAIBotInventoryDigestedSkillSet>();
	if (InventoryDigested)
	{
		UClass* SkillSetClass = This->InventorySkillSet.Get();
		UFortAthenaAIBotInventorySkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotInventorySkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestInventorySkillSet(SkillSet, InventoryDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotLootingDigestedSkillSet* LootingDigested = RuntimeParameters->Cast<UFortAthenaAIBotLootingDigestedSkillSet>();
	if (LootingDigested)
	{
		UClass* SkillSetClass = This->LootingSkillSet.Get();
		UFortAthenaAIBotLootingSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotLootingSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestLootingSkillSet(SkillSet, LootingDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotMovementDigestedSkillSet* MovementDigested = RuntimeParameters->Cast<UFortAthenaAIBotMovementDigestedSkillSet>();
	if (MovementDigested)
	{
		UClass* SkillSetClass = This->MovementSkillSet.Get();
		UFortAthenaAIBotMovementSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotMovementSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestMovementSkillSet(SkillSet, MovementDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotPerceptionDigestedSkillSet* PerceptionDigested = RuntimeParameters->Cast<UFortAthenaAIBotPerceptionDigestedSkillSet>();
	if (PerceptionDigested)
	{
		UClass* SkillSetClass = This->PerceptionSkillSet.Get();
		UFortAthenaAIBotPerceptionSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotPerceptionSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestPerceptionSkillSet(SkillSet, PerceptionDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotPlayStyleDigestedSkillSet* PlayStyleDigested = RuntimeParameters->Cast<UFortAthenaAIBotPlayStyleDigestedSkillSet>();
	if (PlayStyleDigested)
	{
		UClass* SkillSetClass = This->PlayStyleSkillSet.Get();
		UFortAthenaAIBotPlayStyleSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotPlayStyleSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestPlayStyleSkillSet(SkillSet, PlayStyleDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotPropagateAwarenessDigestedSkillSet* PropagateAwarenessDigested = RuntimeParameters->Cast<UFortAthenaAIBotPropagateAwarenessDigestedSkillSet>();
	if (PropagateAwarenessDigested)
	{
		UClass* SkillSetClass = This->PropagateAwarenessSkillSet.Get();
		UFortAthenaAIBotPropagateAwarenessSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotPropagateAwarenessSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestPropagateAwarenessSkillSet(SkillSet, PropagateAwarenessDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotRangeAttackDigestedSkillSet* RangeAttackDigested = RuntimeParameters->Cast<UFortAthenaAIBotRangeAttackDigestedSkillSet>();
	if (RangeAttackDigested)
	{
		UClass* SkillSetClass = This->RangeAttackSkillSet.Get();
		UFortAthenaAIBotRangeAttackSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotRangeAttackSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestRangeAttackSkillSet(SkillSet, RangeAttackDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotReviveDigestedSkillSet* ReviveDigested = RuntimeParameters->Cast<UFortAthenaAIBotReviveDigestedSkillSet>();
	if (ReviveDigested)
	{
		UClass* SkillSetClass = This->ReviveSkillSet.Get();
		UFortAthenaAIBotReviveSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotReviveSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestReviveSkillSet(SkillSet, ReviveDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotUnstuckDigestedSkillSet* UnstuckDigested = RuntimeParameters->Cast<UFortAthenaAIBotUnstuckDigestedSkillSet>();
	if (UnstuckDigested)
	{
		UClass* SkillSetClass = This->UnstuckSkillSet.Get();
		UFortAthenaAIBotUnstuckSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotUnstuckSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestUnstuckSkillSet(SkillSet, UnstuckDigested, SkillLevel);
		}

		return CRC;
	}

	UFortAthenaAIBotWarmupDigestedSkillSet* WarmupDigested = RuntimeParameters->Cast<UFortAthenaAIBotWarmupDigestedSkillSet>();
	if (WarmupDigested)
	{
		UFortAthenaAISpawnerDataComponent_PlayerBotSkillset* PlayerBotThis = This->Cast<UFortAthenaAISpawnerDataComponent_PlayerBotSkillset>();
		UClass* SkillSetClass = PlayerBotThis ? PlayerBotThis->WarmUpSkillSet.Get() : nullptr;
		UFortAthenaAIBotWarmupSkillSet* SkillSet = SkillSetClass && SkillSetClass->DefaultObject ? SkillSetClass->DefaultObject->Cast<UFortAthenaAIBotWarmupSkillSet>() : nullptr;
		if (SkillSet)
		{
			DigestWarmupSkillSet(SkillSet, WarmupDigested, SkillLevel);
		}

		return CRC;
	}

	return CRC;
}

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::Init()
{
	void** ProviderVTable = reinterpret_cast<void**>(ImageBase + 0x6D50BA0);
	Memory::SwapVTableEntry(ProviderVTable, 3, GetAIRuntimeParametersClassesHook);
	Memory::SwapVTableEntry(ProviderVTable, 4, ExtractHook);
}

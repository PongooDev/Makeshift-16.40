#include "pch.h"

void AFortSafeZoneIndicator::SetShrinkDuration(float NewShrinkDuration) {
	SafeZoneFinishShrinkTime = SafeZoneStartShrinkTime + NewShrinkDuration;
}

void AFortSafeZoneIndicator::OffsetShrinkTimesFromServerTime(float TimeOffset) {
	UWorld* World = GetWorld();
	AGameStateBase* GameState = World ? World->GameState : nullptr;
	if (!GameState) {
		return;
	}

	const float ShrinkDuration = FMath::Max(SafeZoneFinishShrinkTime - SafeZoneStartShrinkTime, 0.f);
	SafeZoneStartShrinkTime = GameState->GetServerWorldTimeSeconds() + TimeOffset;
	SafeZoneFinishShrinkTime = SafeZoneStartShrinkTime + ShrinkDuration;
}

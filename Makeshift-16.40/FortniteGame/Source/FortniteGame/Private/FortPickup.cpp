#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/FortPickup.h"

FFortPickupCreationData::FFortPickupCreationData(UWorld* InWorld, const FFortItemEntry& InPickupDataItemEntry, const FVector& InPosition, const FRotator& InRotation, AFortPlayerController* InOptionalPCOwner, TSubclassOf<AFortPickup> InOverrideClass, AActor* InOwnerContainer)
	: Position(InPosition)
	, Rotation(InRotation) {
	void (*Fn)(FFortPickupCreationData*, UWorld*, const FFortItemEntry*, const FVector*, const FRotator*, AFortPlayerController*, UClass*, AActor*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x413BB64);
	Fn(this, InWorld, &InPickupDataItemEntry, &InPosition, &InRotation, InOptionalPCOwner, InOverrideClass, InOwnerContainer);
}

AFortPickup* AFortPickup::CreateFromData(const FFortPickupCreationData& CreationData) {
	AFortPickup* (*Fn)(const FFortPickupCreationData*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x4A51ED0);
	return Fn(&CreationData);
}

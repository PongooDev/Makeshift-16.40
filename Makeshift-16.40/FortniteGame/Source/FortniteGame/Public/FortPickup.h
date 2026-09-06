#include "pch.h"

struct FFortPickupCreationData
{
	UWorld* World;
	const FFortItemEntry PickupDataItemEntry;
	const FVector Position;
	const FRotator Rotation;
	TWeakObjectPtr<AFortPlayerController> OptionalPCOwner;
	TSubclassOf<AFortPickup> OverrideClass;
	TWeakObjectPtr<AActor> OwnerContainer;
	EFortPickupSourceTypeFlag SourceTypeFlags;
	EFortPickupSpawnSource SpawnSource;
	uint8 bRandomRotation : 1;
	uint8 bPickupOnlyRelevantToOwner : 1;

	FFortPickupCreationData(UWorld* InWorld, const FFortItemEntry& InPickupDataItemEntry, const FVector& InPosition, const FRotator& InRotation, AFortPlayerController* InOptionalPCOwner, TSubclassOf<AFortPickup> InOverrideClass, AActor* InOwnerContainer);
};
static_assert(alignof(FFortPickupCreationData) == 0x000008, "Wrong alignment on FFortPickupCreationData");
static_assert(sizeof(FFortPickupCreationData) == 0x0001D0, "Wrong size on FFortPickupCreationData");
static_assert(offsetof(FFortPickupCreationData, World) == 0x000000, "Member 'FFortPickupCreationData::World' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, PickupDataItemEntry) == 0x000008, "Member 'FFortPickupCreationData::PickupDataItemEntry' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, Position) == 0x000198, "Member 'FFortPickupCreationData::Position' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, Rotation) == 0x0001A4, "Member 'FFortPickupCreationData::Rotation' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, OptionalPCOwner) == 0x0001B0, "Member 'FFortPickupCreationData::OptionalPCOwner' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, OverrideClass) == 0x0001B8, "Member 'FFortPickupCreationData::OverrideClass' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, OwnerContainer) == 0x0001C0, "Member 'FFortPickupCreationData::OwnerContainer' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, SourceTypeFlags) == 0x0001C8, "Member 'FFortPickupCreationData::SourceTypeFlags' has a wrong offset!");
static_assert(offsetof(FFortPickupCreationData, SpawnSource) == 0x0001C9, "Member 'FFortPickupCreationData::SpawnSource' has a wrong offset!");

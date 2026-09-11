#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

static const float PoiVolumeHalfHeight = 50000.f;

static TWeakObjectPtr<UWorld> CachedBuildingFoundationsWorld;
static TArray<AActor*> CachedBuildingFoundations;

static const TArray<AActor*>& GetBuildingFoundations(UWorld* World)
{
	if (CachedBuildingFoundationsWorld.Get() != World || CachedBuildingFoundations.Num() <= 0)
	{
		CachedBuildingFoundations.Clear();
		UGameplayStatics::GetAllActorsOfClass(World, ABuildingFoundation::StaticClass(), &CachedBuildingFoundations);
		CachedBuildingFoundationsWorld = TWeakObjectPtr<UWorld>(World);
	}

	return CachedBuildingFoundations;
}

static void AddPointToBounds(FVector& Min, FVector& Max, const FVector& Point, bool& bInitialized)
{
	if (!bInitialized)
	{
		Min = Point;
		Max = Point;
		bInitialized = true;
		return;
	}

	Min = FVector(FMath::Min(Min.X, Point.X), FMath::Min(Min.Y, Point.Y), FMath::Min(Min.Z, Point.Z));
	Max = FVector(FMath::Max(Max.X, Point.X), FMath::Max(Max.Y, Point.Y), FMath::Max(Max.Z, Point.Z));
}

bool AFortPoiVolume::GetLocationFromChallengeMapPoiData(FVector& OutLocation) const
{
	UFortQuestIndicatorData* QuestIndicatorData = UFortQuestIndicatorData::Get();
	if (!QuestIndicatorData)
	{
		return false;
	}

	const FFortChallengeMapPoiData* FoundPoiData = nullptr;
	for (int32 Index = 0; Index < QuestIndicatorData->ChallengeMapPoiData.Num(); ++Index)
	{
		const FFortChallengeMapPoiData& PoiData = QuestIndicatorData->ChallengeMapPoiData[Index];
		if (!PoiData.LocationTag.IsValid() || !LocationTags.HasTagExact(PoiData.LocationTag))
		{
			continue;
		}

		if (PoiData.CalendarEventsRequired.Num() == 0)
		{
			FoundPoiData = &PoiData;
			break;
		}

		if (!FoundPoiData)
		{
			FoundPoiData = &PoiData;
		}
	}

	if (!FoundPoiData)
	{
		return false;
	}

	FVector MapLocationTextLocationOffset(0.f, 0.f, 0.f);
	UActorComponent* Component = GetComponentByClass(UFortPoi_DiscoverableComponent::StaticClass());
	const UFortPoi_DiscoverableComponent* DiscoverableComponent = Component ? Component->Cast<UFortPoi_DiscoverableComponent>() : nullptr;
	if (DiscoverableComponent)
	{
		MapLocationTextLocationOffset = DiscoverableComponent->MapLocationTextLocationOffset;
	}

	OutLocation = FVector(FoundPoiData->WorldLocation.X - MapLocationTextLocationOffset.X, FoundPoiData->WorldLocation.Y - MapLocationTextLocationOffset.Y, FoundPoiData->WorldLocation.Z - MapLocationTextLocationOffset.Z);
	return true;
}

bool AFortPoiVolume::GetBrushBounds(FVector& OutLocalOrigin, FVector& OutExtent) const
{
	if (!Brush)
	{
		return false;
	}

	const FBoxSphereBounds& BrushBounds = Brush->Bounds;
	if (BrushBounds.BoxExtent.X <= 0.f || BrushBounds.BoxExtent.Y <= 0.f || BrushBounds.BoxExtent.Z <= 0.f)
	{
		return false;
	}

	OutLocalOrigin = BrushBounds.Origin;
	OutExtent = BrushBounds.BoxExtent;
	return true;
}

bool AFortPoiVolume::GetLocationBoundsFromBuildingFoundations(FVector& OutOrigin, FVector& OutExtent, int32& OutFoundationCount) const
{
	OutFoundationCount = 0;

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector Min(0.f, 0.f, 0.f);
	FVector Max(0.f, 0.f, 0.f);
	bool bInitialized = false;
	const TArray<AActor*>& BuildingFoundations = GetBuildingFoundations(World);
	for (int32 Index = 0; Index < BuildingFoundations.Num(); ++Index)
	{
		ABuildingFoundation* BuildingFoundation = BuildingFoundations[Index] ? BuildingFoundations[Index]->Cast<ABuildingFoundation>() : nullptr;
		if (!BuildingFoundation || !BuildingFoundation->MapLocationTag.IsValid() || !LocationTags.HasTag(BuildingFoundation->MapLocationTag))
		{
			continue;
		}

		if (BuildingFoundation->StreamingBoundingBox.IsValid)
		{
			const FTransform FoundationTransform = BuildingFoundation->GetTransform();
			const FVector& BoxMin = BuildingFoundation->StreamingBoundingBox.min;
			const FVector& BoxMax = BuildingFoundation->StreamingBoundingBox.max;
			for (int32 CornerIndex = 0; CornerIndex < 8; ++CornerIndex)
			{
				const FVector LocalCorner((CornerIndex & 1) ? BoxMax.X : BoxMin.X, (CornerIndex & 2) ? BoxMax.Y : BoxMin.Y, (CornerIndex & 4) ? BoxMax.Z : BoxMin.Z);
				AddPointToBounds(Min, Max, UKismetMathLibrary::TransformLocation(FoundationTransform, LocalCorner), bInitialized);
			}
		}
		else
		{
			FVector FoundationOrigin(0.f, 0.f, 0.f);
			FVector FoundationExtent(0.f, 0.f, 0.f);
			BuildingFoundation->GetActorBounds(false, &FoundationOrigin, &FoundationExtent, true);
			AddPointToBounds(Min, Max, FVector(FoundationOrigin.X - FoundationExtent.X, FoundationOrigin.Y - FoundationExtent.Y, FoundationOrigin.Z - FoundationExtent.Z), bInitialized);
			AddPointToBounds(Min, Max, FVector(FoundationOrigin.X + FoundationExtent.X, FoundationOrigin.Y + FoundationExtent.Y, FoundationOrigin.Z + FoundationExtent.Z), bInitialized);
		}
		++OutFoundationCount;
	}

	if (OutFoundationCount <= 0)
	{
		return false;
	}

	OutOrigin = FVector((Min.X + Max.X) * 0.5f, (Min.Y + Max.Y) * 0.5f, 0.f);
	OutExtent = FVector((Max.X - Min.X) * 0.5f, (Max.Y - Min.Y) * 0.5f, PoiVolumeHalfHeight);
	return true;
}

bool AFortPoiVolume::ReconstructBrushComponent()
{
	if (BrushComponent)
	{
		return true;
	}

	FVector Location(0.f, 0.f, 0.f);
	FVector LocalOrigin(0.f, 0.f, 0.f);
	FVector Extent(0.f, 0.f, 0.f);
	int32 FoundationCount = 0;
	bool bPlacedFromChallengeMapPoiData = false;
	if (GetLocationFromChallengeMapPoiData(Location) && GetBrushBounds(LocalOrigin, Extent))
	{
		bPlacedFromChallengeMapPoiData = true;
	}
	else if (!GetLocationBoundsFromBuildingFoundations(Location, Extent, FoundationCount))
	{
		UE_LOG(LogFortQuest, Warning, TEXT("AFortPoiVolume::PostInitializeComponents (%hs) The brush component is missing from this build and neither the challenge map POI data nor any building foundation carries one of its %d location tag(s), the volume cannot be placed"), GetName().c_str(), LocationTags.GameplayTags.Num());
		return false;
	}

	UBrushComponent* NewBrushComponent = NewObject<UBrushComponent>(this, UBrushComponent::StaticClass(), FName(L"BrushComponent0"));
	if (!NewBrushComponent)
	{
		return false;
	}

	UBoxComponent* BoxComponent = NewObject<UBoxComponent>(NewBrushComponent, UBoxComponent::StaticClass());
	if (!BoxComponent)
	{
		return false;
	}
	BoxComponent->SetBoxExtent(Extent, false);

	UBodySetup* BrushBodySetup = BoxComponent->ShapeBodySetup;
	if (BrushBodySetup && BrushBodySetup->AggGeom.BoxElems.Num() > 0)
	{
		BrushBodySetup->AggGeom.BoxElems[0].Center = LocalOrigin;
	}

	NewBrushComponent->Brush = Brush;
	NewBrushComponent->BrushBodySetup = BrushBodySetup;
	NewBrushComponent->SetCollisionProfileName(FName(L"NoCollision"), false);

	FTransform Transform{};
	Transform.Rotation = FQuat(0.f, 0.f, 0.f, 1.f);
	Transform.Translation = Location;
	Transform.Scale3D = FVector(1.f, 1.f, 1.f);
	NewBrushComponent->K2_SetWorldTransform(Transform, false, nullptr, true);
	NewBrushComponent->SetMobility(EComponentMobility::Static);

	BrushComponent = NewBrushComponent;
	RootComponent = NewBrushComponent;

	NewBrushComponent->RegisterComponentWithWorld(GetWorld());
	NewBrushComponent->Bounds.Origin = FVector(Location.X + LocalOrigin.X, Location.Y + LocalOrigin.Y, Location.Z + LocalOrigin.Z);
	NewBrushComponent->Bounds.BoxExtent = Extent;
	NewBrushComponent->Bounds.SphereRadius = sqrtf(Extent.X * Extent.X + Extent.Y * Extent.Y + Extent.Z * Extent.Z);

	if (PoiCollisionComp)
	{
		PoiCollisionComp->K2_AttachToComponent(NewBrushComponent, FName(), EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	}

	if (bPlacedFromChallengeMapPoiData)
	{
		UE_LOG(LogFortQuest, Log, TEXT("AFortPoiVolume::PostInitializeComponents (%hs) The brush component is missing from this build, rebuilt it at the challenge map POI location %f %f %f from the cooked brush model (origin %f %f %f, extent %f %f %f)"), GetName().c_str(), Location.X, Location.Y, Location.Z, LocalOrigin.X, LocalOrigin.Y, LocalOrigin.Z, Extent.X, Extent.Y, Extent.Z);
	}
	else
	{
		UE_LOG(LogFortQuest, Log, TEXT("AFortPoiVolume::PostInitializeComponents (%hs) The brush component is missing from this build, rebuilt it from %d building foundation(s) at %f %f %f with an extent of %f %f %f"), GetName().c_str(), FoundationCount, Location.X, Location.Y, Location.Z, Extent.X, Extent.Y, Extent.Z);
	}
	return true;
}

void AFortPoiVolume::PostInitializeComponentsHook(AFortPoiVolume* This)
{
	This->ReconstructBrushComponent();

	PostInitializeComponentsOG(This);
}

void AFortPoiVolume::Init()
{
	Memory::HookDetour(ImageBase + 0x10BA310, PostInitializeComponentsHook, &PostInitializeComponentsOG);
}

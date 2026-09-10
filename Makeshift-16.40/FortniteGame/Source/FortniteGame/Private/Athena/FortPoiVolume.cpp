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

bool AFortPoiVolume::GetLocationBoundsFromBuildingFoundations(FVector& OutOrigin, FVector& OutExtent, int32& OutFoundationCount) const
{
	OutFoundationCount = 0;

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector Min;
	FVector Max;
	const TArray<AActor*>& BuildingFoundations = GetBuildingFoundations(World);
	for (int32 Index = 0; Index < BuildingFoundations.Num(); ++Index)
	{
		ABuildingFoundation* BuildingFoundation = BuildingFoundations[Index] ? BuildingFoundations[Index]->Cast<ABuildingFoundation>() : nullptr;
		if (!BuildingFoundation || !BuildingFoundation->MapLocationTag.IsValid() || !LocationTags.HasTag(BuildingFoundation->MapLocationTag))
		{
			continue;
		}

		FVector FoundationMin;
		FVector FoundationMax;
		if (BuildingFoundation->StreamingBoundingBox.IsValid)
		{
			FoundationMin = BuildingFoundation->StreamingBoundingBox.min;
			FoundationMax = BuildingFoundation->StreamingBoundingBox.max;
		}
		else
		{
			FVector FoundationOrigin;
			FVector FoundationExtent;
			BuildingFoundation->GetActorBounds(false, &FoundationOrigin, &FoundationExtent, true);
			FoundationMin = FVector(FoundationOrigin.X - FoundationExtent.X, FoundationOrigin.Y - FoundationExtent.Y, FoundationOrigin.Z - FoundationExtent.Z);
			FoundationMax = FVector(FoundationOrigin.X + FoundationExtent.X, FoundationOrigin.Y + FoundationExtent.Y, FoundationOrigin.Z + FoundationExtent.Z);
		}

		if (OutFoundationCount == 0)
		{
			Min = FoundationMin;
			Max = FoundationMax;
		}
		else
		{
			Min = FVector(FMath::Min(Min.X, FoundationMin.X), FMath::Min(Min.Y, FoundationMin.Y), FMath::Min(Min.Z, FoundationMin.Z));
			Max = FVector(FMath::Max(Max.X, FoundationMax.X), FMath::Max(Max.Y, FoundationMax.Y), FMath::Max(Max.Z, FoundationMax.Z));
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

	FVector Origin;
	FVector Extent;
	int32 FoundationCount = 0;
	if (!GetLocationBoundsFromBuildingFoundations(Origin, Extent, FoundationCount))
	{
		UE_LOG(LogFortQuest, Warning, TEXT("AFortPoiVolume::PostInitializeComponents (%hs) The brush component is missing from this build and no building foundation carries any of its %d location tag(s), the volume cannot be placed"), GetName().c_str(), LocationTags.GameplayTags.Num());
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

	NewBrushComponent->Brush = Brush;
	NewBrushComponent->BrushBodySetup = BoxComponent->ShapeBodySetup;
	NewBrushComponent->SetCollisionProfileName(FName(L"NoCollision"), false);

	FTransform Transform{};
	Transform.Rotation = FQuat(0.f, 0.f, 0.f, 1.f);
	Transform.Translation = Origin;
	Transform.Scale3D = FVector(1.f, 1.f, 1.f);
	NewBrushComponent->K2_SetWorldTransform(Transform, false, nullptr, true);
	NewBrushComponent->SetMobility(EComponentMobility::Static);

	BrushComponent = NewBrushComponent;
	RootComponent = NewBrushComponent;

	NewBrushComponent->RegisterComponentWithWorld(GetWorld());
	NewBrushComponent->Bounds.Origin = Origin;
	NewBrushComponent->Bounds.BoxExtent = Extent;
	NewBrushComponent->Bounds.SphereRadius = sqrtf(Extent.X * Extent.X + Extent.Y * Extent.Y + Extent.Z * Extent.Z);

	if (PoiCollisionComp)
	{
		PoiCollisionComp->K2_AttachToComponent(NewBrushComponent, FName(), EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	}

	const FVector CollisionLocation = PoiCollisionComp ? PoiCollisionComp->K2_GetComponentLocation() : FVector();
	UE_LOG(LogFortQuest, Log, TEXT("AFortPoiVolume::PostInitializeComponents (%hs) The brush component is missing from this build, rebuilt it from %d building foundation(s) at %f %f %f with an extent of %f %f %f, collision component at %f %f %f"), GetName().c_str(), FoundationCount, Origin.X, Origin.Y, Origin.Z, Extent.X, Extent.Y, Extent.Z, CollisionLocation.X, CollisionLocation.Y, CollisionLocation.Z);
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

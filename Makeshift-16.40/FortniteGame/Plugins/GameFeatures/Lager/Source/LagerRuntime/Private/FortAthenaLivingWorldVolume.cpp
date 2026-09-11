#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include <vector>

struct FPendingEnvironmentQuery
{
	TWeakObjectPtr<AFortAthenaLivingWorldVolume> Volume;
	TWeakObjectPtr<UEnvQueryInstanceBlueprintWrapper> QueryInstance;
	float StartTime;
};

static std::vector<FPendingEnvironmentQuery> PendingEnvironmentQueries;

struct FDeferredEnvironmentQuery
{
	TWeakObjectPtr<AFortAthenaLivingWorldVolume> Volume;
	int32 Attempts;
	float NextAttemptTime;
	int32 GridPointsPerSide;
	bool bWaitingForNavigation;
};

static std::vector<FDeferredEnvironmentQuery> DeferredEnvironmentQueries;

static const int32 MaxEnvironmentQueryAttempts = 8;

static const int32 MinEnvironmentQueryGridPointsPerSide = 48;

static const int32 MaxEnvironmentQueryGridPointsPerSide = 256;

static const int32 MinEnvironmentQueryPoints = 64;

static const float EnvironmentQueryRetryDelay = 10.f;

static FDeferredEnvironmentQuery* DeferEnvironmentQuery(AFortAthenaLivingWorldVolume* Volume, float TimeSeconds, bool bCountAttempt)
{
	FDeferredEnvironmentQuery* DeferredQuery = nullptr;
	for (size_t Index = 0; Index < DeferredEnvironmentQueries.size(); ++Index)
	{
		if (DeferredEnvironmentQueries[Index].Volume.Get() == Volume)
		{
			DeferredQuery = &DeferredEnvironmentQueries[Index];
			break;
		}
	}

	if (!DeferredQuery)
	{
		FDeferredEnvironmentQuery NewDeferredQuery;
		NewDeferredQuery.Volume = TWeakObjectPtr<AFortAthenaLivingWorldVolume>(Volume);
		NewDeferredQuery.Attempts = 0;
		NewDeferredQuery.NextAttemptTime = 0.f;
		NewDeferredQuery.GridPointsPerSide = 0;
		NewDeferredQuery.bWaitingForNavigation = false;
		DeferredEnvironmentQueries.push_back(NewDeferredQuery);
		DeferredQuery = &DeferredEnvironmentQueries.back();
	}

	DeferredQuery->NextAttemptTime = TimeSeconds + EnvironmentQueryRetryDelay;
	if (bCountAttempt)
	{
		++DeferredQuery->Attempts;
	}

	return DeferredQuery;
}

static int32 GetEnvironmentQueryGridPointsPerSide(const AFortAthenaLivingWorldVolume* Volume)
{
	for (size_t Index = 0; Index < DeferredEnvironmentQueries.size(); ++Index)
	{
		if (DeferredEnvironmentQueries[Index].Volume.Get() == Volume && DeferredEnvironmentQueries[Index].GridPointsPerSide > 0)
		{
			return DeferredEnvironmentQueries[Index].GridPointsPerSide;
		}
	}

	return MinEnvironmentQueryGridPointsPerSide;
}

static void PostponeDeferredEnvironmentQuery(const AFortAthenaLivingWorldVolume* Volume, float TimeSeconds)
{
	for (size_t Index = 0; Index < DeferredEnvironmentQueries.size(); ++Index)
	{
		if (DeferredEnvironmentQueries[Index].Volume.Get() == Volume)
		{
			DeferredEnvironmentQueries[Index].NextAttemptTime = TimeSeconds + EnvironmentQueryRetryDelay;
			return;
		}
	}
}

static void RemoveDeferredEnvironmentQuery(const AFortAthenaLivingWorldVolume* Volume)
{
	for (size_t Index = 0; Index < DeferredEnvironmentQueries.size();)
	{
		AFortAthenaLivingWorldVolume* DeferredVolume = DeferredEnvironmentQueries[Index].Volume.Get();
		if (!DeferredVolume || DeferredVolume == Volume)
		{
			DeferredEnvironmentQueries.erase(DeferredEnvironmentQueries.begin() + Index);
			continue;
		}

		++Index;
	}
}

struct FReconstructedVolumeBounds
{
	TWeakObjectPtr<AFortAthenaLivingWorldVolume> Volume;
	FVector Extent;
};

static std::vector<FReconstructedVolumeBounds> ReconstructedVolumeBounds;

static bool FindReconstructedVolumeExtent(const AFortAthenaLivingWorldVolume* Volume, FVector& OutExtent)
{
	for (size_t Index = 0; Index < ReconstructedVolumeBounds.size(); ++Index)
	{
		if (ReconstructedVolumeBounds[Index].Volume.Get() == Volume)
		{
			OutExtent = ReconstructedVolumeBounds[Index].Extent;
			return true;
		}
	}
	return false;
}

static FPendingEnvironmentQuery* FindPendingEnvironmentQuery(const AFortAthenaLivingWorldVolume* Volume)
{
	for (size_t Index = 0; Index < PendingEnvironmentQueries.size(); ++Index)
	{
		if (PendingEnvironmentQueries[Index].Volume.Get() == Volume)
		{
			return &PendingEnvironmentQueries[Index];
		}
	}
	return nullptr;
}

static void RemovePendingEnvironmentQuery(const AFortAthenaLivingWorldVolume* Volume)
{
	for (size_t Index = 0; Index < PendingEnvironmentQueries.size();)
	{
		if (PendingEnvironmentQueries[Index].Volume.Get() == Volume)
		{
			PendingEnvironmentQueries.erase(PendingEnvironmentQueries.begin() + Index);
		}
		else
		{
			++Index;
		}
	}
}

static void ShieldQueryInstanceFromGarbageCollection(UEnvQueryInstanceBlueprintWrapper* QueryInstance, bool bShield)
{
	if (!QueryInstance)
	{
		return;
	}

	FUObjectItem* ObjectItem = UObject::GObjects->IndexToObject(QueryInstance->Index);
	if (!ObjectItem)
	{
		return;
	}

	if (bShield)
	{
		ObjectItem->Flags |= int32(EInternalObjectFlags::RootSet);
	}
	else
	{
		ObjectItem->Flags &= ~int32(EInternalObjectFlags::RootSet);
	}
}

struct FVolumeGridGeneratorSettings
{
	float GridSize;
	float SpaceBetween;
	float ProjectDown;
	float ProjectUp;
	int32 TraceMode;

	FVolumeGridGeneratorSettings()
		: GridSize(0.f)
		, SpaceBetween(0.f)
		, ProjectDown(0.f)
		, ProjectUp(0.f)
		, TraceMode(0)
	{
	}
};

static bool RequiresVolumeBounds(UEnvQuery* EnvironmentQuery)
{
	if (!EnvironmentQuery)
	{
		return false;
	}

	for (int32 OptionIndex = 0; OptionIndex < EnvironmentQuery->Options.Num(); ++OptionIndex)
	{
		UEnvQueryOption* Option = EnvironmentQuery->Options[OptionIndex];
		UEnvQueryGenerator* Generator = Option ? Option->Generator : nullptr;
		if (Generator && Generator->IsA(UEnvQueryGenerator_SimpleGrid::StaticClass()))
		{
			return true;
		}
	}

	return false;
}

static int32 RemoveVolumeContainmentTests(UEnvQuery* EnvironmentQuery)
{
	int32 RemovedCount = 0;
	if (!EnvironmentQuery)
	{
		return RemovedCount;
	}

	for (int32 OptionIndex = 0; OptionIndex < EnvironmentQuery->Options.Num(); ++OptionIndex)
	{
		UEnvQueryOption* Option = EnvironmentQuery->Options[OptionIndex];
		if (!Option)
		{
			continue;
		}

		for (int32 TestIndex = Option->Tests.Num() - 1; TestIndex >= 0; --TestIndex)
		{
			UEnvQueryTest* Test = Option->Tests[TestIndex];
			if (Test && Test->IsA(UEnvQueryTest_Volume::StaticClass()))
			{
				Option->Tests.RemoveAt(TestIndex, 1, false);
				++RemovedCount;
			}
		}
	}

	return RemovedCount;
}

static bool GetPlayableBounds(AFortGameStateAthena* GameState, FVector& OutOrigin, FVector& OutExtent)
{
	AFortAthenaMapInfo* MapInfo = GameState ? GameState->MapInfo : nullptr;
	if (!MapInfo)
	{
		return false;
	}

	OutOrigin = MapInfo->CachedPlayableBoundsForClients.Origin;
	OutExtent = MapInfo->CachedPlayableBoundsForClients.BoxExtent;
	if (OutExtent.X > 0.f && OutExtent.Y > 0.f)
	{
		return true;
	}

	if (AVolume* PlayableBoundsVolume = MapInfo->PlayableBoundsVolume)
	{
		PlayableBoundsVolume->GetActorBounds(false, &OutOrigin, &OutExtent, false);
		if (OutExtent.X > 0.f && OutExtent.Y > 0.f)
		{
			return true;
		}
	}

	const FBox2D& AircraftDropZone = MapInfo->AircraftDropZone;
	if (AircraftDropZone.max.X > AircraftDropZone.min.X && AircraftDropZone.max.Y > AircraftDropZone.min.Y)
	{
		OutOrigin = FVector((AircraftDropZone.min.X + AircraftDropZone.max.X) * 0.5f, (AircraftDropZone.min.Y + AircraftDropZone.max.Y) * 0.5f, 0.f);
		OutExtent = FVector((AircraftDropZone.max.X - AircraftDropZone.min.X) * 0.5f, (AircraftDropZone.max.Y - AircraftDropZone.min.Y) * 0.5f, 0.f);
		return true;
	}

	return false;
}

static float GetGroundReferenceZ(AFortAthenaLivingWorldVolume* Volume, float DefaultZ)
{
	UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(Volume);
	if (!LivingWorldManager)
	{
		return DefaultZ;
	}

	float TotalZ = 0.f;
	int32 Count = 0;
	for (int32 Index = 0; Index < LivingWorldManager->PointProviders.Num(); ++Index)
	{
		UObject* Object = LivingWorldManager->PointProviders[Index].ObjectPointer;
		AActor* PointProviderActor = Object ? Object->Cast<AActor>() : nullptr;
		if (!PointProviderActor || !PointProviderActor->IsA(AFortAthenaPatrolPathPointProvider::StaticClass()))
		{
			continue;
		}

		TotalZ += PointProviderActor->GetActorLocation().Z;
		++Count;
	}

	return Count > 0 ? TotalZ / float(Count) : DefaultZ;
}

struct FOriginalGridGeneratorSettings
{
	TWeakObjectPtr<UEnvQueryGenerator_SimpleGrid> Generator;
	float GridSize;
};

static std::vector<FOriginalGridGeneratorSettings> OriginalGridGeneratorSettings;

static float GetOriginalGridSize(UEnvQueryGenerator_SimpleGrid* SimpleGrid)
{
	for (size_t Index = 0; Index < OriginalGridGeneratorSettings.size(); ++Index)
	{
		if (OriginalGridGeneratorSettings[Index].Generator.Get() == SimpleGrid)
		{
			return OriginalGridGeneratorSettings[Index].GridSize;
		}
	}

	FOriginalGridGeneratorSettings Original;
	Original.Generator = TWeakObjectPtr<UEnvQueryGenerator_SimpleGrid>(SimpleGrid);
	Original.GridSize = SimpleGrid->GridSize.DefaultValue;
	OriginalGridGeneratorSettings.push_back(Original);

	return Original.GridSize;
}

static void ApplyVolumeBoundsToGridGenerator(const AFortAthenaLivingWorldVolume* Volume, UEnvQuery* EnvironmentQuery, int32 GridPointsPerSide, FVolumeGridGeneratorSettings& OutSettings)
{
	if (!Volume || !EnvironmentQuery)
	{
		return;
	}

	FVector BoxExtent;
	if (!FindReconstructedVolumeExtent(Volume, BoxExtent))
	{
		FVector Origin;
		const_cast<AFortAthenaLivingWorldVolume*>(Volume)->GetActorBounds(false, &Origin, &BoxExtent, false);
	}

	float GridSize = FMath::Max(BoxExtent.X, BoxExtent.Y);
	if (GridSize <= 0.f)
	{
		GridSize = 30000.f;
	}

	for (int32 OptionIndex = 0; OptionIndex < EnvironmentQuery->Options.Num(); ++OptionIndex)
	{
		UEnvQueryOption* Option = EnvironmentQuery->Options[OptionIndex];
		UEnvQueryGenerator* Generator = Option ? Option->Generator : nullptr;
		if (!Generator || !Generator->IsA(UEnvQueryGenerator_SimpleGrid::StaticClass()))
		{
			continue;
		}

		UEnvQueryGenerator_SimpleGrid* SimpleGrid = static_cast<UEnvQueryGenerator_SimpleGrid*>(Generator);
		SimpleGrid->GridSize.DataBinding = nullptr;
		SimpleGrid->GridSize.DefaultValue = FMath::Max(GridSize, GetOriginalGridSize(SimpleGrid));

		SimpleGrid->SpaceBetween.DataBinding = nullptr;
		SimpleGrid->SpaceBetween.DefaultValue = (SimpleGrid->GridSize.DefaultValue * 2.f) / float(GridPointsPerSide - 1);

		OutSettings.GridSize = SimpleGrid->GridSize.DefaultValue;
		OutSettings.SpaceBetween = SimpleGrid->SpaceBetween.DefaultValue;
		OutSettings.ProjectDown = SimpleGrid->ProjectionData.ProjectDown;
		OutSettings.ProjectUp = SimpleGrid->ProjectionData.ProjectUp;
		OutSettings.TraceMode = static_cast<int32>(SimpleGrid->ProjectionData.TraceMode);
	}
}

static AFortGameStateAthena* GetFortGameStateAthena(UWorld* World)
{
	return World && World->GameState ? World->GameState->Cast<AFortGameStateAthena>() : nullptr;
}

bool AFortAthenaLivingWorldVolume::IsRegisteredToLivingWorldManager() const
{
	UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(const_cast<AFortAthenaLivingWorldVolume*>(this));
	if (!LivingWorldManager)
	{
		return false;
	}

	for (int32 Index = 0; Index < LivingWorldManager->PointProviders.Num(); ++Index)
	{
		if (LivingWorldManager->PointProviders[Index].ObjectPointer == this)
		{
			return true;
		}
	}
	return false;
}

void AFortAthenaLivingWorldVolume::OnCurrentPlaylistLoaded(FName PlaylistName, const FGameplayTagContainer& PlaylistContextTags)
{
	CachedGameState = GetFortGameStateAthena(GetWorld());
	if (Role != ENetRole::ROLE_Authority || !CachedGameState)
	{
		return;
	}

	EnablePointProvider();
}

void AFortAthenaLivingWorldVolume::EnablePointProvider()
{
	bIsEnabled = true;
	if (!IsRegisteredToLivingWorldManager() && !FindPendingEnvironmentQuery(this))
	{
		RunEQS();
	}
}

void AFortAthenaLivingWorldVolume::DisablePointProvider()
{
	bIsEnabled = false;
	RemovePendingEnvironmentQuery(this);
	if (UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(this))
	{
		LivingWorldManager->UnregisterPointProvider(this);
	}
}

bool AFortAthenaLivingWorldVolume::ReconstructVolumeRootComponent()
{
	FVector Origin;
	FVector Extent;
	if (RootComponent || !EnvironmentQuery || !GetPlayableBounds(CachedGameState, Origin, Extent))
	{
		return false;
	}

	Origin.Z = GetGroundReferenceZ(this, Origin.Z);

	USceneComponent* SceneComponent = NewObject<USceneComponent>(this, USceneComponent::StaticClass());
	if (!SceneComponent)
	{
		return false;
	}

	SceneComponent->SetMobility(EComponentMobility::Movable);
	RootComponent = SceneComponent;
	K2_SetActorLocation(Origin, false, nullptr, true);

	FReconstructedVolumeBounds Bounds;
	Bounds.Volume = TWeakObjectPtr<AFortAthenaLivingWorldVolume>(this);
	Bounds.Extent = Extent;
	ReconstructedVolumeBounds.push_back(Bounds);

	const int32 RemovedTestCount = RemoveVolumeContainmentTests(EnvironmentQuery);
	const FVector ActorLocation = GetActorLocation();
	UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::ReconstructVolumeRootComponent (%hs) The brush component is missing from this build, a root component was created at %f %f %f with an extent of %f %f, %d volume containment test(s) removed from %hs"), GetName().c_str(), ActorLocation.X, ActorLocation.Y, ActorLocation.Z, Extent.X, Extent.Y, RemovedTestCount, EnvironmentQuery->GetName().c_str());
	return true;
}

void AFortAthenaLivingWorldVolume::RunEQS()
{
	if (!EnvironmentQuery)
	{
		UE_LOG(LogLivingWorldManager, Warning, TEXT("AFortAthenaLivingWorldVolume::RunEQS (%hs) No environment query set, disabling volume"), GetName().c_str());
		bIsEnabled = false;
		return;
	}

	if (!RootComponent && RequiresVolumeBounds(EnvironmentQuery) && !ReconstructVolumeRootComponent())
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::RunEQS (%hs) has no brush component in this build and the playable bounds are not available yet, the environment query will be started later"), GetName().c_str());
		return;
	}

	FVolumeGridGeneratorSettings GridGeneratorSettings;
	ApplyVolumeBoundsToGridGenerator(this, EnvironmentQuery, GetEnvironmentQueryGridPointsPerSide(this), GridGeneratorSettings);
	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, EnvironmentQuery, this, EEnvQueryRunMode::AllMatching, UEnvQueryInstanceBlueprintWrapper::StaticClass());
	if (!QueryInstance)
	{
		UE_LOG(LogLivingWorldManager, Warning, TEXT("AFortAthenaLivingWorldVolume::RunEQS (%hs) The environment query could not be started"), GetName().c_str());
		return;
	}

	EQSRequestID = QueryInstance->QueryID;
	ShieldQueryInstanceFromGarbageCollection(QueryInstance, true);

	UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::RunEQS (%hs) started query %hs id %d, grid size %f, %f between points, projecting %f down and %f up in trace mode %d, %d pending"), GetName().c_str(), EnvironmentQuery->GetName().c_str(), EQSRequestID, GridGeneratorSettings.GridSize, GridGeneratorSettings.SpaceBetween, GridGeneratorSettings.ProjectDown, GridGeneratorSettings.ProjectUp, GridGeneratorSettings.TraceMode, static_cast<int32>(PendingEnvironmentQueries.size()) + 1);

	RemovePendingEnvironmentQuery(this);
	FPendingEnvironmentQuery PendingQuery;
	PendingQuery.Volume = TWeakObjectPtr<AFortAthenaLivingWorldVolume>(this);
	PendingQuery.QueryInstance = TWeakObjectPtr<UEnvQueryInstanceBlueprintWrapper>(QueryInstance);
	PendingQuery.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	PendingEnvironmentQueries.push_back(PendingQuery);
}

void AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries()
{
	if (DeferredEnvironmentQueries.size() > 0)
	{
		std::vector<FDeferredEnvironmentQuery> DeferredQueries = DeferredEnvironmentQueries;
		for (size_t Index = 0; Index < DeferredQueries.size(); ++Index)
		{
			AFortAthenaLivingWorldVolume* DeferredVolume = DeferredQueries[Index].Volume.Get();
			if (!DeferredVolume)
			{
				RemoveDeferredEnvironmentQuery(nullptr);
				continue;
			}

			if (!DeferredVolume->bIsEnabled || FindPendingEnvironmentQuery(DeferredVolume))
			{
				continue;
			}

			UWorld* DeferredWorld = DeferredVolume->GetWorld();
			const float DeferredTimeSeconds = DeferredWorld ? DeferredWorld->GetTimeSeconds() : 0.f;
			if (DeferredTimeSeconds < DeferredQueries[Index].NextAttemptTime)
			{
				continue;
			}

			PostponeDeferredEnvironmentQuery(DeferredVolume, DeferredTimeSeconds);
			DeferredVolume->RunEQS();
		}
	}

	static size_t LastReportedPendingCount = 0;
	if (PendingEnvironmentQueries.size() != LastReportedPendingCount)
	{
		LastReportedPendingCount = PendingEnvironmentQueries.size();
		UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries : %d environment query/queries pending"), static_cast<int32>(LastReportedPendingCount));
	}

	for (size_t Index = 0; Index < PendingEnvironmentQueries.size();)
	{
		AFortAthenaLivingWorldVolume* Volume = PendingEnvironmentQueries[Index].Volume.Get();
		UEnvQueryInstanceBlueprintWrapper* QueryInstance = PendingEnvironmentQueries[Index].QueryInstance.Get();
		if (!Volume)
		{
			ShieldQueryInstanceFromGarbageCollection(QueryInstance, false);
			PendingEnvironmentQueries.erase(PendingEnvironmentQueries.begin() + Index);
			continue;
		}

		if (!QueryInstance)
		{
			UE_LOG(LogLivingWorldManager, Warning, TEXT("AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries (%hs) The environment query instance was destroyed before it finished"), Volume->GetName().c_str());
			Volume->EQSRequestID = -1;
			PendingEnvironmentQueries.erase(PendingEnvironmentQueries.begin() + Index);
			continue;
		}

		TArray<FVector> QueryLocations;
		if (!QueryInstance->GetQueryResultsAsLocations(&QueryLocations))
		{
			UWorld* World = Volume->GetWorld();
			const float TimeSeconds = World ? World->GetTimeSeconds() : 0.f;
			UClass* FinishedItemType = QueryInstance->ItemType;
			const bool bFinished = FinishedItemType != nullptr;
			if (bFinished || TimeSeconds - PendingEnvironmentQueries[Index].StartTime > 180.f)
			{
				Volume->EQSRequestID = -1;
				ShieldQueryInstanceFromGarbageCollection(QueryInstance, false);
				PendingEnvironmentQueries.erase(PendingEnvironmentQueries.begin() + Index);

				UNavigationSystemV1* NavigationSystem = World && World->NavigationSystem ? World->NavigationSystem->Cast<UNavigationSystemV1>() : nullptr;
				const bool bNavigationBeingBuilt = NavigationSystem && NavigationSystem->IsNavigationBuildInProgress();
				FDeferredEnvironmentQuery* DeferredQuery = DeferEnvironmentQuery(Volume, TimeSeconds, !bNavigationBeingBuilt);
				if (bNavigationBeingBuilt)
				{
					if (!DeferredQuery->bWaitingForNavigation)
					{
						DeferredQuery->bWaitingForNavigation = true;
						UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries (%hs) The navigation data is still being built, the environment query will keep being started until it is ready"), Volume->GetName().c_str());
					}

					continue;
				}

				if (DeferredQuery->Attempts < MaxEnvironmentQueryAttempts)
				{
					UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries (%hs) The environment query %hs, attempt %d of %d, starting it again in %f second(s)"), Volume->GetName().c_str(), bFinished ? "found no point to spawn at" : "did not finish in time", DeferredQuery->Attempts, MaxEnvironmentQueryAttempts, EnvironmentQueryRetryDelay);
					continue;
				}

				UE_LOG(LogLivingWorldManager, Warning, TEXT("AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries (%hs) The environment query returned no usable result, disabling volume"), Volume->GetName().c_str());
				Volume->bIsEnabled = false;
				RemoveDeferredEnvironmentQuery(Volume);
				continue;
			}

			++Index;
			continue;
		}

		ShieldQueryInstanceFromGarbageCollection(QueryInstance, false);
		PendingEnvironmentQueries.erase(PendingEnvironmentQueries.begin() + Index);

		const int32 GridPointsPerSide = GetEnvironmentQueryGridPointsPerSide(Volume);
		if (QueryLocations.Num() < MinEnvironmentQueryPoints && GridPointsPerSide < MaxEnvironmentQueryGridPointsPerSide)
		{
			UWorld* VolumeWorld = Volume->GetWorld();
			const float VolumeTimeSeconds = VolumeWorld ? VolumeWorld->GetTimeSeconds() : 0.f;
			FDeferredEnvironmentQuery* DeferredQuery = DeferEnvironmentQuery(Volume, VolumeTimeSeconds, false);
			DeferredQuery->GridPointsPerSide = FMath::Min(GridPointsPerSide * 2, MaxEnvironmentQueryGridPointsPerSide);

			UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries (%hs) The environment query only computed %d point(s), starting it again with %d grid point(s) per side"), Volume->GetName().c_str(), QueryLocations.Num(), DeferredQuery->GridPointsPerSide);
			continue;
		}

		RemoveDeferredEnvironmentQuery(Volume);
		Volume->OnEnvQueryFinished(QueryLocations);
	}
}

void AFortAthenaLivingWorldVolume::OnEnvQueryFinished(const TArray<FVector>& QueryLocations)
{
	EQSRequestID = -1;

	const FVector ActorLocation = GetActorLocation();
	EnvironmentQueryResults.Clear();
	PointProviderMaxRadiusSqr = 0.f;
	for (int32 Index = 0; Index < QueryLocations.Num(); ++Index)
	{
		const FVector& QueryLocation = QueryLocations[Index];
		EnvironmentQueryResults.Add(QueryLocation);
		const float DistanceSqr = FVector::DistSquaredXY(ActorLocation, QueryLocation);
		if (DistanceSqr > PointProviderMaxRadiusSqr)
		{
			PointProviderMaxRadiusSqr = DistanceSqr;
		}
	}
	CachedSafeZoneFilteredResultsSourceCount = 0;

	if (EnvironmentQueryResults.Num() <= 0)
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::OnEnvQueryFinished (%hs) No point computed with the EQS query, disabling volume"), GetName().c_str());
		bIsEnabled = false;
		return;
	}

	UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(this);
	if (GetWorld() && LivingWorldManager)
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::OnEnvQueryFinished (%hs) %d point(s) computed with the EQS query"), GetName().c_str(), EnvironmentQueryResults.Num());
		LivingWorldManager->RegisterPointProvider(this);
	}
}

void AFortAthenaLivingWorldVolume::GetPointsInsideSafeZone(const FVector& SafeZoneCenter, float SafeZoneRadiusSqr, TArray<FVector>& FilteredPoints) const
{
	FilteredPoints.Clear();
	for (int32 Index = 0; Index < EnvironmentQueryResults.Num(); ++Index)
	{
		const FVector& Point = EnvironmentQueryResults[Index];
		if (FVector::DistSquaredXY(SafeZoneCenter, Point) <= SafeZoneRadiusSqr)
		{
			FilteredPoints.Add(Point);
		}
	}
}

const TArray<FVector>* AFortAthenaLivingWorldVolume::GetSafeZoneFilteredPoints(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const
{
	if (!PointFilter.bOnlyPointsInSafeZone || !CachedGameState)
	{
		return &EnvironmentQueryResults;
	}

	const IFortSafeZoneInterface* SafeZoneInterface = CachedGameState->GetSafeZoneInterface();
	if (!SafeZoneInterface->GetSafeZoneState())
	{
		return &EnvironmentQueryResults;
	}

	const FVector SafeZoneCenter = SafeZoneInterface->GetSafeZoneCenter();
	const float SafeZoneRadius = SafeZoneInterface->GetSafeZoneRadius();
	const float SafeZoneRadiusSqr = SafeZoneRadius * SafeZoneRadius;

	AFortAthenaLivingWorldVolume* MutableThis = const_cast<AFortAthenaLivingWorldVolume*>(this);
	if (CachedSafeZoneFilteredResultsSourceCount != EnvironmentQueryResults.Num()
		|| CachedSafeZoneFilteredResultsRadiusSqr != SafeZoneRadiusSqr
		|| CachedSafeZoneFilteredResultsCenter.X != SafeZoneCenter.X
		|| CachedSafeZoneFilteredResultsCenter.Y != SafeZoneCenter.Y
		|| CachedSafeZoneFilteredResultsCenter.Z != SafeZoneCenter.Z)
	{
		GetPointsInsideSafeZone(SafeZoneCenter, SafeZoneRadiusSqr, MutableThis->CachedSafeZoneFilteredResults);
		MutableThis->CachedSafeZoneFilteredResultsCenter = SafeZoneCenter;
		MutableThis->CachedSafeZoneFilteredResultsRadiusSqr = SafeZoneRadiusSqr;
		MutableThis->CachedSafeZoneFilteredResultsSourceCount = EnvironmentQueryResults.Num();
	}

	return &CachedSafeZoneFilteredResults;
}

bool AFortAthenaLivingWorldVolume::GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, const float ActorDensityComputationRadius, FVector& OutPosition, FRotator& OutRotation) const
{
	if (!bIsEnabled || !CachedGameState || EnvironmentQueryResults.Num() <= 0)
	{
		return false;
	}

	const TArray<FVector>* CandidatePoints = GetSafeZoneFilteredPoints(PointFilter);

	if (PointFilter.bCheckForActorDensity)
	{
		const UFortAthenaLivingWorldManager* LivingWorldManager = UFortAthenaLivingWorldManager::GetLivingWorldManager(const_cast<AFortAthenaLivingWorldVolume*>(this));
		AFortAthenaLivingWorldVolume* MutableThis = const_cast<AFortAthenaLivingWorldVolume*>(this);
		MutableThis->CachedDensityFilteredResults.Clear();
		for (int32 Index = 0; Index < CandidatePoints->Num(); ++Index)
		{
			const FVector& Point = (*CandidatePoints)[Index];
			if (IFortAthenaLivingWorldPointProviderInterface::IsPointWithinDensityRules(PointFilter, Point, LivingWorldManager, ActorDensityComputationRadius))
			{
				MutableThis->CachedDensityFilteredResults.Add(Point);
			}
		}
		CandidatePoints = &CachedDensityFilteredResults;
	}

	if (CandidatePoints->Num() <= 0)
	{
		return false;
	}

	OutPosition = (*CandidatePoints)[FMath::RandHelper(CandidatePoints->Num())];
	OutRotation = FRotator(0.f, FMath::FRand() * 360.f, 0.f);
	return true;
}

bool AFortAthenaLivingWorldVolume::IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const
{
	if (!bIsEnabled || !CachedGameState || EnvironmentQueryResults.Num() <= 0)
	{
		return false;
	}

	return GetSafeZoneFilteredPoints(PointFilter)->Num() > 0;
}

void AFortAthenaLivingWorldVolume::OnPointProviderRegistered(const TScriptInterface<IFortAthenaLivingWorldPointProviderInterface>& PointProvider)
{
	AActor* PointProviderActor = PointProvider.ObjectPointer ? PointProvider.ObjectPointer->Cast<AActor>() : nullptr;
	const IFortAthenaLivingWorldPointProviderInterface* PointProviderInterface = static_cast<const IFortAthenaLivingWorldPointProviderInterface*>(PointProvider.InterfacePointer);
	if (!PointProviderActor || !PointProviderInterface || PointProviderActor == this || DeactivationTagsQuery.IsEmpty())
	{
		return;
	}

	FGameplayTagContainer FilterTags;
	PointProviderInterface->GetFiltersTags(FilterTags);
	if (!DeactivationTagsQuery.Matches(FilterTags))
	{
		return;
	}

	const float Distance = sqrtf(FVector::DistSquaredXY(GetActorLocation(), PointProviderActor->GetActorLocation()));
	if (Distance < DeactivationTagsRange)
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::OnPointProviderRegistered (%hs) Disabled by the registration of %hs"), GetName().c_str(), PointProviderActor->GetName().c_str());
		DisablePointProvider();
	}
}

void AFortAthenaLivingWorldVolume::OnCurrentPlaylistLoadedForAllVolumes(UWorld* World, FName PlaylistName, const FGameplayTagContainer& PlaylistContextTags)
{
	if (!World)
	{
		return;
	}

	TArray<AActor*> Volumes;
	UGameplayStatics::GetAllActorsOfClass(World, AFortAthenaLivingWorldVolume::StaticClass(), &Volumes);
	UE_LOG(LogLivingWorldManager, Log, TEXT("AFortAthenaLivingWorldVolume::OnCurrentPlaylistLoaded : %d living world volume(s) in the world"), Volumes.Num());

	for (int32 Index = 0; Index < Volumes.Num(); ++Index)
	{
		AFortAthenaLivingWorldVolume* Volume = Volumes[Index] ? Volumes[Index]->Cast<AFortAthenaLivingWorldVolume>() : nullptr;
		if (Volume)
		{
			Volume->OnCurrentPlaylistLoaded(PlaylistName, PlaylistContextTags);
		}
	}
}

void AFortAthenaLivingWorldVolume::EnablePendingVolumes(UFortAthenaLivingWorldManager* LivingWorldManager)
{
	UWorld* World = LivingWorldManager ? LivingWorldManager->CachedWorld : nullptr;
	if (!World)
	{
		return;
	}

	TArray<AActor*> Volumes;
	UGameplayStatics::GetAllActorsOfClass(World, AFortAthenaLivingWorldVolume::StaticClass(), &Volumes);

	for (int32 Index = 0; Index < Volumes.Num(); ++Index)
	{
		AFortAthenaLivingWorldVolume* Volume = Volumes[Index] ? Volumes[Index]->Cast<AFortAthenaLivingWorldVolume>() : nullptr;
		if (!Volume || Volume->Role != ENetRole::ROLE_Authority || !Volume->bIsEnabled)
		{
			continue;
		}

		if (Volume->IsRegisteredToLivingWorldManager() || FindPendingEnvironmentQuery(Volume))
		{
			continue;
		}

		if (!Volume->CachedGameState)
		{
			Volume->CachedGameState = GetFortGameStateAthena(World);
		}
		Volume->EnablePointProvider();
	}
}

bool AFortAthenaLivingWorldVolume::GetValidLocationHook(const IFortAthenaLivingWorldPointProviderInterface* This, const FFortAthenaLivingWorldPointProviderFilterRules* PointFilter, float ActorDensityComputationRadius, FVector* OutPosition, FRotator* OutRotation)
{
	const AFortAthenaLivingWorldVolume* Volume = reinterpret_cast<const AFortAthenaLivingWorldVolume*>(reinterpret_cast<const uint8*>(This) - 0x258);
	return Volume->GetValidLocation(*PointFilter, ActorDensityComputationRadius, *OutPosition, *OutRotation);
}

bool AFortAthenaLivingWorldVolume::IsEnabledHook(const IFortAthenaLivingWorldPointProviderInterface* This, const FFortAthenaLivingWorldPointProviderFilterRules* PointFilter)
{
	const AFortAthenaLivingWorldVolume* Volume = reinterpret_cast<const AFortAthenaLivingWorldVolume*>(reinterpret_cast<const uint8*>(This) - 0x258);
	return Volume->IsEnabled(*PointFilter);
}

DEFINE_FUNCTION(AFortAthenaLivingWorldVolume::execOnCurrentPlaylistLoaded)
{
	P_GET_PROPERTY(FNameProperty,Z_Param_PlaylistName);
	P_GET_STRUCT_REF(FGameplayTagContainer,Z_Param_Out_PlaylistContextTags);
	P_FINISH;
	P_THIS_CAST(AFortAthenaLivingWorldVolume)->OnCurrentPlaylistLoaded(Z_Param_PlaylistName,Z_Param_Out_PlaylistContextTags);
}

DEFINE_FUNCTION(AFortAthenaLivingWorldVolume::execOnPointProviderRegistered)
{
	P_GET_TINTERFACE_REF(IFortAthenaLivingWorldPointProviderInterface,Z_Param_Out_PointProvider);
	P_FINISH;
	P_THIS_CAST(AFortAthenaLivingWorldVolume)->OnPointProviderRegistered(Z_Param_Out_PointProvider);
}

void AFortAthenaLivingWorldVolume::Init()
{
	Memory::HookDetour(ImageBase + 0x3C80234, (void*)&AFortAthenaLivingWorldVolume::execOnCurrentPlaylistLoaded);
	Memory::HookDetour(ImageBase + 0x3C80964, (void*)&AFortAthenaLivingWorldVolume::execOnPointProviderRegistered);
	Memory::SwapVTableEntryInAllSubClasses<AFortAthenaLivingWorldVolume>(2, GetValidLocationHook, 0x258);
	Memory::SwapVTableEntryInAllSubClasses<AFortAthenaLivingWorldVolume>(7, IsEnabledHook, 0x258);
}

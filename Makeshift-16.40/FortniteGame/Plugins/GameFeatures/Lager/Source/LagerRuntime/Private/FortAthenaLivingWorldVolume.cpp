#include "pch.h"
#include <vector>

struct FPendingEnvironmentQuery
{
	TWeakObjectPtr<AFortAthenaLivingWorldVolume> Volume;
	TWeakObjectPtr<UEnvQueryInstanceBlueprintWrapper> QueryInstance;
};

static std::vector<FPendingEnvironmentQuery> PendingEnvironmentQueries;

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

void AFortAthenaLivingWorldVolume::RunEQS()
{
	if (!EnvironmentQuery)
	{
		UE_LOG(LogLivingWorldManager, Warning, TEXT("AFortAthenaLivingWorldVolume::RunEQS (%hs) No environment query set, disabling volume"), GetName().c_str());
		bIsEnabled = false;
		return;
	}

	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, EnvironmentQuery, this, EEnvQueryRunMode::AllMatching, UEnvQueryInstanceBlueprintWrapper::StaticClass());
	if (!QueryInstance)
	{
		UE_LOG(LogLivingWorldManager, Warning, TEXT("AFortAthenaLivingWorldVolume::RunEQS (%hs) The environment query could not be started"), GetName().c_str());
		return;
	}

	EQSRequestID = QueryInstance->QueryID;
	ShieldQueryInstanceFromGarbageCollection(QueryInstance, true);

	if (EQSRequestID == -1)
	{
		UE_LOG(LogLivingWorldManager, Warning, TEXT("AFortAthenaLivingWorldVolume::RunEQS (%hs) The environment query did not start"), GetName().c_str());
	}

	RemovePendingEnvironmentQuery(this);
	FPendingEnvironmentQuery PendingQuery;
	PendingQuery.Volume = TWeakObjectPtr<AFortAthenaLivingWorldVolume>(this);
	PendingQuery.QueryInstance = TWeakObjectPtr<UEnvQueryInstanceBlueprintWrapper>(QueryInstance);
	PendingEnvironmentQueries.push_back(PendingQuery);
}

void AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries()
{
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
			++Index;
			continue;
		}

		ShieldQueryInstanceFromGarbageCollection(QueryInstance, false);
		PendingEnvironmentQueries.erase(PendingEnvironmentQueries.begin() + Index);
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

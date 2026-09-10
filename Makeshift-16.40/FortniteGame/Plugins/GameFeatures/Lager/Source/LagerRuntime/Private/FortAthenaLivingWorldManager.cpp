#include "pch.h"
#include "FortniteGame/Source/FortniteGame/Public/FortAssets.h"

static void (*InitializeOG)(UFortAthenaLivingWorldManager* This) = nullptr;
static std::unordered_map<int32, FVector> SpawnRequestPositions;

static float DistSquared(const FVector& A, const FVector& B)
{
	return (A - B).SizeSquared();
}

UFortAthenaLivingWorldManager* UFortAthenaLivingWorldManager::GetLivingWorldManager(UObject* WorldContextObject)
{
	UWorld* World = UWorld::GetWorld();
	AGameStateBase* GameState = World ? World->GameState : nullptr;
	return GameState ? GameState->FindComponentByClass<UFortAthenaLivingWorldManager>() : nullptr;
}

void UFortAthenaLivingWorldManager::GetAllCategoryRows(const UDataTable* Table, TArray<FFortAthenaLivingWorldCategory*>& OutRows)
{
	void (*Fn)(const UDataTable*, const wchar_t*, TArray<FFortAthenaLivingWorldCategory*>*) = decltype(Fn)(ImageBase + 0x3C78A90);
	Fn(Table, L"UFortAthenaLivingWorldManager::GetAllCategories", &OutRows);
}

void UFortAthenaLivingWorldManager::GetAllEventRows(const UDataTable* Table, TArray<FFortAthenaLivingWorldEvent*>& OutRows)
{
	void (*Fn)(const UDataTable*, const wchar_t*, TArray<FFortAthenaLivingWorldEvent*>*) = decltype(Fn)(ImageBase + 0x3C78C54);
	Fn(Table, L"UFortAthenaLivingWorldManager::GetActiveEvents", &OutRows);
}

bool UFortAthenaLivingWorldManager::IsLivingWorldEnabled() const
{
	bool (*Fn)(const UFortAthenaLivingWorldManager*) = decltype(Fn)(ImageBase + 0x3C7BDB0);
	return Fn(this);
}

uint8 UFortAthenaLivingWorldManager::GetSafeZonePhase() const
{
	uint8 (*Fn)(const UFortAthenaLivingWorldManager*) = decltype(Fn)(ImageBase + 0x3C7ACC4);
	return Fn(this);
}

float UFortAthenaLivingWorldManager::GetArrayValueForPhase(const TArray<float>& Values, uint8 SafeZonePhase) const
{
	float (*Fn)(const UFortAthenaLivingWorldManager*, const TArray<float>*, uint32) = decltype(Fn)(ImageBase + 0x3C7ACDC);
	return Fn(this, &Values, SafeZonePhase);
}

int32 UFortAthenaLivingWorldManager::GetCategoryCurrentCount(const FFortAthenaLivingWorldCategory* Category) const
{
	int32 (*Fn)(const UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldCategory*) = decltype(Fn)(ImageBase + 0x3C7AD74);
	return Fn(this, Category);
}

int32 UFortAthenaLivingWorldManager::GetEventCurrentCount(const FFortAthenaLivingWorldEvent* Event) const
{
	int32 (*Fn)(const UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*) = decltype(Fn)(ImageBase + 0x3C7ADD8);
	return Fn(this, Event);
}

int32 UFortAthenaLivingWorldManager::GetTotalActorCount() const
{
	int32 (*Fn)(const UFortAthenaLivingWorldManager*) = decltype(Fn)(ImageBase + 0x3C7AD00);
	return Fn(this);
}

int32 UFortAthenaLivingWorldManager::GetEventBudget(const FFortAthenaLivingWorldEvent* Event, const FLivingWorldEventRuntimeData* EventRuntimeData, uint8 SafeZonePhase) const
{
	int32 (*Fn)(const UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*, const FLivingWorldEventRuntimeData*, uint32) = decltype(Fn)(ImageBase + 0x3C7AA48);
	return Fn(this, Event, EventRuntimeData, SafeZonePhase);
}

bool UFortAthenaLivingWorldManager::IsEventActive(const FFortAthenaLivingWorldEvent* Event, const UFortAthenaLivingWorldEventData* EventData, bool bSkipActivationPhaseValidation, EAthenaGamePhaseStep GamePhaseStep, uint8 SafeZonePhase) const
{
	bool (*Fn)(const UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*, const UFortAthenaLivingWorldEventData*, bool, uint8, uint32) = decltype(Fn)(ImageBase + 0x3C7BD38);
	return Fn(this, Event, EventData, bSkipActivationPhaseValidation, static_cast<uint8>(GamePhaseStep), SafeZonePhase);
}

UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData* UFortAthenaLivingWorldManager::FindEventRuntimeData(const FFortAthenaLivingWorldEvent* Event)
{
	FLivingWorldEventRuntimeData* (*Fn)(UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*) = decltype(Fn)(ImageBase + 0x3C7A098);
	return Fn(this, Event);
}

UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData* UFortAthenaLivingWorldManager::FindOrCreateEventRuntimeData(const FFortAthenaLivingWorldEvent* Event)
{
	FLivingWorldEventRuntimeData* (*Fn)(UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*) = decltype(Fn)(ImageBase + 0x3C7A3DC);
	return Fn(this, Event);
}

float UFortAthenaLivingWorldManager::GetDensityComputationDistance() const
{
	float (*Fn)(const UFortAthenaLivingWorldManager*) = decltype(Fn)(ImageBase + 0x3C7A988);
	return Fn(this);
}

float UFortAthenaLivingWorldManager::GetMaxActorDensity() const
{
	return CachedConfig ? CachedConfig->MaxActorDensity.GetValueAtLevel(0.f) : 0.f;
}

float UFortAthenaLivingWorldManager::GetActorDensity(const FVector& Location, float Radius) const
{
	const float RadiusSqr = Radius * Radius;
	int32 Density = 0;
	for (int32 Index = 0; Index < SpawnedPawns.Num(); ++Index)
	{
		AActor* SpawnedPawn = SpawnedPawns[Index];
		if (SpawnedPawn && DistSquared(SpawnedPawn->GetActorLocation(), Location) <= RadiusSqr)
		{
			++Density;
		}
	}
	return static_cast<float>(Density);
}

void UFortAthenaLivingWorldManager::Initialize()
{
	if (!PrimaryComponentTick.bCanEverTick)
	{
		PrimaryComponentTick.bCanEverTick = true;
		RegisterComponentTickFunctions(true);
	}

	AActor* Owner = GetOwner();
	if (!CachedWorld)
	{
		CachedWorld = Owner ? Owner->GetWorld() : nullptr;
	}

	if (!CachedGameState)
	{
		AGameStateBase* GameState = CachedWorld ? CachedWorld->GameState : nullptr;
		CachedGameState = GameState ? GameState->Cast<AFortGameStateAthena>() : nullptr;
	}

	InitializeOG(this);

	UE_LOG(LogLivingWorldManager, Log, TEXT("UFortAthenaLivingWorldManager::Initialize : Enabled %d, Config %hs, World %hs, GameState %hs, PointProviders %d, NextEventGenerationTime %f"), IsLivingWorldEnabled() ? 1 : 0, CachedConfig ? CachedConfig->GetName().c_str() : "None", CachedWorld ? CachedWorld->GetName().c_str() : "None", CachedGameState ? CachedGameState->GetName().c_str() : "None", PointProviders.Num(), NextEventGenerationTime);
}

void UFortAthenaLivingWorldManager::InitializeHook(UFortAthenaLivingWorldManager* This)
{
	This->Initialize();
}

void UFortAthenaLivingWorldManager::TickComponentHook(UFortAthenaLivingWorldManager* This, float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	This->TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFortAthenaLivingWorldManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	void (*SuperTickComponent)(UActorComponent*, float, ELevelTick, FActorComponentTickFunction*) = decltype(SuperTickComponent)(ImageBase + 0x10D4B68);
	SuperTickComponent(this, DeltaTime, TickType, ThisTickFunction);

	if (!CachedWorld || !CachedConfig || !CachedGameState)
	{
		return;
	}

	AFortAthenaLivingWorldVolume::ProcessPendingEnvironmentQueries();
	ReleaseInstances();

	const float TimeSeconds = CachedWorld->GetTimeSeconds();
	if (bToggleGenerateEvents && NextEventGenerationTime >= 0.f && TimeSeconds >= NextEventGenerationTime)
	{
		NextEventGenerationTime = TimeSeconds + CachedConfig->MinimumEventGenerationInterval.GetValueAtLevel(0.f);
		EnableNewPointProviders();
		GenerateEvents();
	}

	ProcessEventRequests();
}

void UFortAthenaLivingWorldManager::GatherPatrolPaths()
{
	void (*Fn)(UFortAthenaLivingWorldManager*) = decltype(Fn)(ImageBase + 0x3C7A50C);
	Fn(this);
}

void UFortAthenaLivingWorldManager::EnableNewPointProviders()
{
	GatherPatrolPaths();
	AFortAthenaLivingWorldVolume::EnablePendingVolumes(this);
}

void UFortAthenaLivingWorldManager::GetAllCategories(TArray<FFortAthenaLivingWorldCategory*>& OutCategories) const
{
	if (!CachedConfig)
	{
		return;
	}

	if (UDataTable* CategoryTable = FFortAssets::GetAsset(CachedConfig->CategoryTable))
	{
		GetAllCategoryRows(CategoryTable, OutCategories);
	}
}

void UFortAthenaLivingWorldManager::GenerateEvents()
{
	if (!CachedConfig)
	{
		return;
	}

	if (bVerboseLogging)
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("UFortAthenaLivingWorldManager::GenerateEvents : %d point provider(s), %d actor(s) alive of %d, %d spawner data class(es) loaded"), PointProviders.Num(), GetTotalActorCount(), static_cast<int32>(CachedConfig->MaxActorCount.GetValueAtLevel(0.f)), LoadedSpawnerDataClass.Num());
	}

	if (static_cast<int32>(CachedConfig->MaxActorCount.GetValueAtLevel(0.f)) - GetTotalActorCount() > 0)
	{
		TArray<FFortAthenaLivingWorldCategory*> Categories;
		GetAllCategories(Categories);
		GenerateEventsFromCategories(Categories);
	}
}

void UFortAthenaLivingWorldManager::GenerateEventsFromCategories(const TArray<FFortAthenaLivingWorldCategory*>& Categories)
{
	TArray<FFortAthenaLivingWorldCategory*> ActiveCategories;
	for (int32 Index = 0; Index < Categories.Num(); ++Index)
	{
		FFortAthenaLivingWorldCategory* Category = Categories[Index];
		if (Category && Category->RequirePlaylistTagQuery.Matches(CachedPlaylistContextTags) && FFortAssets::GetAsset(Category->EventTable))
		{
			ActiveCategories.Add(Category);
		}
	}

	for (int32 Index = 1; Index < ActiveCategories.Num(); ++Index)
	{
		FFortAthenaLivingWorldCategory* Category = ActiveCategories[Index];
		int32 InsertIndex = Index;
		while (InsertIndex > 0 && ActiveCategories[InsertIndex - 1]->Priority < Category->Priority)
		{
			ActiveCategories[InsertIndex] = ActiveCategories[InsertIndex - 1];
			--InsertIndex;
		}
		ActiveCategories[InsertIndex] = Category;
	}

	for (int32 Index = 0; Index < ActiveCategories.Num(); ++Index)
	{
		ProcessCategory(ActiveCategories[Index]);
	}
}

void UFortAthenaLivingWorldManager::ProcessCategory(const FFortAthenaLivingWorldCategory* Category)
{
	if (!Category || !CachedConfig)
	{
		return;
	}

	const UDataTable* EventTable = FFortAssets::GetAsset(Category->EventTable);
	if (!EventTable)
	{
		return;
	}

	GenerateEventRequests(Category, EventTable, ComputeCategoryRemainingBudget(Category));
}

int32 UFortAthenaLivingWorldManager::ComputeCategoryRemainingBudget(const FFortAthenaLivingWorldCategory* Category) const
{
	if (!Category || !CachedConfig)
	{
		return 0;
	}

	const int32 GlobalRemainingBudget = FMath::Max(0, static_cast<int32>(CachedConfig->MaxActorCount.GetValueAtLevel(0.f)) - GetTotalActorCount());
	int32 RemainingBudget = FMath::Max(0, Category->MaxCount - GetCategoryCurrentCount(Category));
	if (!Category->bExcludeFromGlobalAIBudget)
	{
		RemainingBudget = FMath::Min(GlobalRemainingBudget, RemainingBudget);
	}
	return RemainingBudget;
}

void UFortAthenaLivingWorldManager::ComputeActiveEvents(const UDataTable* EventTable, TArray<const FFortAthenaLivingWorldEvent*>& ActiveEvents, float& ActiveEventsTotalWeight)
{
	ActiveEvents.Clear();
	ActiveEventsTotalWeight = 0.f;

	if (!EventTable || !CachedWorld || !CachedGameState)
	{
		return;
	}

	TArray<FFortAthenaLivingWorldEvent*> Events;
	GetAllEventRows(EventTable, Events);

	const uint8 SafeZonePhase = GetSafeZonePhase();
	const EAthenaGamePhaseStep GamePhaseStep = CachedGameState->GamePhaseStep;
	const float TimeSeconds = CachedWorld->GetTimeSeconds();

	for (int32 Index = 0; Index < Events.Num(); ++Index)
	{
		const FFortAthenaLivingWorldEvent* Event = Events[Index];
		if (!Event)
		{
			continue;
		}

		const UFortAthenaLivingWorldEventData* EventData = FFortAssets::GetAsset(Event->EventData);
		if (!IsEventActive(Event, EventData, false, GamePhaseStep, SafeZonePhase))
		{
			continue;
		}

		FLivingWorldEventRuntimeData* EventRuntimeData = FindOrCreateEventRuntimeData(Event);
		if (!EventRuntimeData)
		{
			continue;
		}

		if (EventRuntimeData->ActivationTime == -1.f)
		{
			EventRuntimeData->ActivationTime = TimeSeconds + FMath::RandRange(Event->ActivationMinDelay, Event->ActivationMaxDelay);
		}

		if (TimeSeconds >= EventRuntimeData->ActivationTime)
		{
			ActiveEvents.Add(Event);
			ActiveEventsTotalWeight += Event->Weight;
		}
	}
}

const FFortAthenaLivingWorldEvent* UFortAthenaLivingWorldManager::GetRandomEventFromEvents(const TArray<const FFortAthenaLivingWorldEvent*>& ActiveEvents, float ActiveEventsTotalWeight) const
{
	if (ActiveEvents.Num() == 0)
	{
		return nullptr;
	}

	const float RandomWeight = FMath::FRand() * ActiveEventsTotalWeight;
	float AccumulatedWeight = 0.f;
	for (int32 Index = 0; Index < ActiveEvents.Num(); ++Index)
	{
		const FFortAthenaLivingWorldEvent* Event = ActiveEvents[Index];
		if (!Event)
		{
			continue;
		}

		AccumulatedWeight += Event->Weight;
		if (RandomWeight <= AccumulatedWeight)
		{
			return Event;
		}
	}

	return ActiveEvents[ActiveEvents.Num() - 1];
}

TScriptInterface<IFortAthenaLivingWorldPointProviderInterface> UFortAthenaLivingWorldManager::GetRandomPointProviderFromEventRuntimeData(const FLivingWorldEventRuntimeData& EventRuntimeData, const FFortAthenaLivingWorldPointProviderFilterRules& ProviderFilterRules) const
{
	TScriptInterface<IFortAthenaLivingWorldPointProviderInterface> Result;
	Result.ObjectPointer = nullptr;
	Result.InterfacePointer = nullptr;

	const FGameplayTagQuery* ProviderQuery = nullptr;
	const float RandomWeight = FMath::FRand() * EventRuntimeData.TotalProviderWeight;
	float AccumulatedWeight = 0.f;
	for (int32 Index = 0; Index < EventRuntimeData.ProviderFiltersEntries.Num(); ++Index)
	{
		const FPointProviderFilterEntry& Entry = EventRuntimeData.ProviderFiltersEntries[Index];
		AccumulatedWeight += Entry.Weight.GetValueAtLevel(0.f);
		if (RandomWeight <= AccumulatedWeight)
		{
			ProviderQuery = &Entry.ProviderFiltersTagQuery;
			break;
		}
	}

	TArray<TScriptInterface<IFortAthenaLivingWorldPointProviderInterface>> ValidPointProviders;
	for (int32 Index = 0; Index < EventRuntimeData.MatchingPointProviders.Num(); ++Index)
	{
		const TScriptInterface<IFortAthenaLivingWorldPointProviderInterface>& PointProvider = EventRuntimeData.MatchingPointProviders[Index];
		if (!PointProvider.ObjectPointer || !PointProvider.InterfacePointer)
		{
			continue;
		}

		const IFortAthenaLivingWorldPointProviderInterface* PointProviderInterface = static_cast<const IFortAthenaLivingWorldPointProviderInterface*>(PointProvider.InterfacePointer);
		FGameplayTagContainer Tags;
		PointProviderInterface->GetFiltersTags(Tags);
		if ((!ProviderQuery || ProviderQuery->IsEmpty() || ProviderQuery->Matches(Tags)) && PointProviderInterface->IsEnabled(ProviderFilterRules))
		{
			ValidPointProviders.Add(PointProvider);
		}
	}

	if (ValidPointProviders.Num() > 0)
	{
		Result = ValidPointProviders[FMath::RandHelper(ValidPointProviders.Num())];
	}
	return Result;
}

void UFortAthenaLivingWorldManager::GenerateEventRequests(const FFortAthenaLivingWorldCategory* Category, const UDataTable* EventTable, int32 RemainingBudget)
{
	if (!Category || !EventTable || RemainingBudget <= 0)
	{
		return;
	}

	const uint8 SafeZonePhase = GetSafeZonePhase();

	TArray<const FFortAthenaLivingWorldEvent*> ActiveEvents;
	float ActiveEventsTotalWeight = 0.f;
	ComputeActiveEvents(EventTable, ActiveEvents, ActiveEventsTotalWeight);

	while (RemainingBudget > 0 && ActiveEvents.Num() > 0)
	{
		const FFortAthenaLivingWorldEvent* Event = GetRandomEventFromEvents(ActiveEvents, ActiveEventsTotalWeight);
		if (!Event)
		{
			break;
		}

		bool bEventUsable = false;
		FLivingWorldEventRuntimeData* EventRuntimeData = FindOrCreateEventRuntimeData(Event);
		const UFortAthenaLivingWorldEventData* EventData = FFortAssets::GetAsset(Event->EventData);
		if (EventRuntimeData && EventData && EventData->ActorDescriptions.Num() > 0)
		{
			const int32 EventBudget = FMath::Min(GetEventBudget(Event, EventRuntimeData, SafeZonePhase), RemainingBudget);
			if (EventBudget > 0)
			{
				FFortAthenaLivingWorldPointProviderFilterRules ProviderFilterRules;
				ProviderFilterRules.bOnlyPointsInSafeZone = Event->bOnlyInSafeZone;
				ProviderFilterRules.bCheckForActorDensity = Event->bCheckForActorDensity;

				const TScriptInterface<IFortAthenaLivingWorldPointProviderInterface> PointProvider = GetRandomPointProviderFromEventRuntimeData(*EventRuntimeData, ProviderFilterRules);
				if (PointProvider.ObjectPointer && PointProvider.InterfacePointer)
				{
					const int32 MinActorCount = FMath::Max(1, static_cast<int32>(EventData->MinActorCountToSpawn.GetValueAtLevel(0.f)));
					const int32 MaxActorCount = FMath::Max(MinActorCount, static_cast<int32>(EventData->MaxActorCountToSpawn.GetValueAtLevel(0.f)));
					const int32 ActorCount = FMath::Min(FMath::RandRange(MinActorCount, MaxActorCount), FMath::Min(EventBudget, EventData->ActorDescriptions.Num()));
					if (ActorCount > 0)
					{
						FLivingWorldEventRequest EventRequest{};
						EventRequest.Category = Category;
						EventRequest.Event = Event;
						EventRequest.PointProvider = static_cast<AActor*>(PointProvider.ObjectPointer);
						EventRequest.ActorCount = ActorCount;
						PendingEventRequests.Add(EventRequest);

						EventRuntimeData->GameSpawnedCount += ActorCount;
						static_cast<IFortAthenaLivingWorldPointProviderInterface*>(PointProvider.InterfacePointer)->OnEventRequested();
						RemainingBudget -= ActorCount;
						bEventUsable = true;

						if (bVerboseLogging)
						{
							UE_LOG(LogLivingWorldManager, Log, TEXT("UFortAthenaLivingWorldManager::GenerateEventRequests : Requested %d actor(s) of event %hs on point provider %hs"), ActorCount, EventData->GetName().c_str(), EventRequest.PointProvider->GetName().c_str());
						}
					}
				}
			}
		}

		if (!bEventUsable)
		{
			for (int32 Index = 0; Index < ActiveEvents.Num(); ++Index)
			{
				if (ActiveEvents[Index] == Event)
				{
					ActiveEventsTotalWeight -= Event->Weight;
					ActiveEvents.Remove(Index);
					break;
				}
			}
		}
	}
}

void UFortAthenaLivingWorldManager::ProcessEventRequests()
{
	if (!CachedConfig)
	{
		return;
	}

	const int32 MaxEventSpawnPerTick = static_cast<int32>(CachedConfig->MaxEventSpawnPerTick.GetValueAtLevel(0.f));
	const int32 RequestCount = FMath::Min(MaxEventSpawnPerTick, PendingEventRequests.Num());
	if (RequestCount <= 0)
	{
		return;
	}

	for (int32 Index = 0; Index < RequestCount; ++Index)
	{
		CreateSpawnRequestFromEventRequest(PendingEventRequests[Index]);
	}

	for (int32 Index = 0; Index < RequestCount; ++Index)
	{
		PendingEventRequests[0].SpawnRequestIDs.Clear();
		PendingEventRequests.Remove(0);
	}
}

TSubclassOf<UFortAthenaAISpawnerData> UFortAthenaLivingWorldManager::GetSpawnerDataClassFromSpawnDescription(const FFortAthenaLivingWorldEventDataActorSpawnDescription& ActorDescription) const
{
	if (UClass* SpawnerDataClass = FFortAssets::GetSubclassOf(ActorDescription.SpawnerData, false))
	{
		return SpawnerDataClass;
	}

	if (ActorDescription.SpawnerDataTagQuery.IsEmpty())
	{
		return nullptr;
	}

	TArray<UClass*> MatchingClasses;
	for (int32 Index = 0; Index < LoadedSpawnerDataClass.Num(); ++Index)
	{
		UClass* LoadedClass = const_cast<TSubclassOf<UFortAthenaAISpawnerData>&>(LoadedSpawnerDataClass[Index]).Get();
		if (!LoadedClass || !LoadedClass->DefaultObject || !LoadedClass->DefaultObject->IsA(UFortAthenaAISpawnerData::StaticClass()))
		{
			continue;
		}

		const UFortAthenaAISpawnerData* SpawnerData = static_cast<const UFortAthenaAISpawnerData*>(LoadedClass->DefaultObject);
		if (ActorDescription.SpawnerDataTagQuery.Matches(SpawnerData->DescriptorTag))
		{
			MatchingClasses.Add(LoadedClass);
		}
	}

	return MatchingClasses.Num() > 0 ? MatchingClasses[FMath::RandHelper(MatchingClasses.Num())] : nullptr;
}

int32 UFortAthenaLivingWorldManager::RequestSpawnToAISpawnerSystem(UAthenaAISpawner& SpawnerSystem, const FTransform& SpawnTransform, const TSubclassOf<UFortAthenaAISpawnerData>& SpawnerData)
{
	if (!const_cast<TSubclassOf<UFortAthenaAISpawnerData>&>(SpawnerData).Get())
	{
		return -1;
	}

	UFortAthenaAISpawnerDataComponentList* ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerData, this);
	if (!ComponentList)
	{
		return -1;
	}

	UAthenaAISpawner::FSpawnRequestCallback Callback(this, &UFortAthenaLivingWorldManager::OnActorSpawnedCallback);
	const int32 RequestID = SpawnerSystem.RequestSpawn(ComponentList, SpawnTransform, Callback);
	Callback.Release();
	return RequestID;
}

void UFortAthenaLivingWorldManager::CreateSpawnRequestFromEventRequest(const FLivingWorldEventRequest& EventRequest)
{
	const FFortAthenaLivingWorldEvent* Event = EventRequest.Event;
	const UFortAthenaLivingWorldEventData* EventData = Event ? FFortAssets::GetAsset(Event->EventData) : nullptr;
	if (!Event || !EventData || !CachedWorld)
	{
		return;
	}

	AActor* PointProviderActor = EventRequest.PointProvider;
	IFortAthenaLivingWorldPointProviderInterface* PointProvider = PointProviderActor ? static_cast<IFortAthenaLivingWorldPointProviderInterface*>(PointProviderActor->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass())) : nullptr;
	FLivingWorldEventRuntimeData* EventRuntimeData = FindOrCreateEventRuntimeData(Event);
	if (!PointProvider)
	{
		if (EventRuntimeData)
		{
			EventRuntimeData->GameSpawnedCount -= EventRequest.ActorCount;
		}
		return;
	}

	UAthenaAISpawner* SpawnerSystem = UFortAIFunctionLibrary::GetAISpawner(CachedWorld);
	if (!SpawnerSystem)
	{
		UE_LOG(LogLivingWorldManager, Warning, TEXT("UFortAthenaLivingWorldManager::CreateSpawnRequestFromEventRequest : No AI spawner system"));
		if (EventRuntimeData)
		{
			EventRuntimeData->GameSpawnedCount -= EventRequest.ActorCount;
		}
		PointProvider->OnSpawnedActorFailed();
		return;
	}

	FFortAthenaLivingWorldPointProviderFilterRules ProviderFilterRules;
	ProviderFilterRules.bOnlyPointsInSafeZone = Event->bOnlyInSafeZone;
	ProviderFilterRules.bCheckForActorDensity = Event->bCheckForActorDensity;
	const float ActorDensityComputationRadius = GetDensityComputationDistance();

	TArray<int32> SpawnRequestIDs;
	const int32 DescriptionCount = EventData->ActorDescriptions.Num();
	for (int32 ActorIndex = 0; ActorIndex < EventRequest.ActorCount; ++ActorIndex)
	{
		const FFortAthenaLivingWorldEventDataActorSpawnDescription& ActorDescription = EventData->ActorDescriptions[FMath::Min(ActorIndex, DescriptionCount - 1)];
		const TSubclassOf<UFortAthenaAISpawnerData> SpawnerDataClass = GetSpawnerDataClassFromSpawnDescription(ActorDescription);
		if (!const_cast<TSubclassOf<UFortAthenaAISpawnerData>&>(SpawnerDataClass).Get())
		{
			if (bVerboseLogging)
			{
				UE_LOG(LogLivingWorldManager, Error, TEXT("UFortAthenaLivingWorldManager::CreateSpawnRequestFromEventRequest : (%hs) Spawn description %d didn't have any spawner data to spawn."), EventData->GetName().c_str(), ActorIndex);
			}
			continue;
		}

		FVector PositionFromProvider;
		FRotator RotationFromProvider;
		if (!PointProvider->GetValidLocation(ProviderFilterRules, ActorDensityComputationRadius, PositionFromProvider, RotationFromProvider))
		{
			if (bVerboseLogging)
			{
				UE_LOG(LogLivingWorldManager, Error, TEXT("UFortAthenaLivingWorldManager::CreateSpawnRequestFromEventRequest : (%hs) PointProvider(%hs) couldn't provide a valid spawning location."), EventData->GetName().c_str(), PointProviderActor->GetName().c_str());
			}
			continue;
		}

		alignas(16) FTransform SpawnTransform(RotationFromProvider, PositionFromProvider, FVector(1.f, 1.f, 1.f));
		const int32 RequestID = RequestSpawnToAISpawnerSystem(*SpawnerSystem, SpawnTransform, SpawnerDataClass);
		if (RequestID == -1)
		{
			UE_LOG(LogLivingWorldManager, Warning, TEXT("UFortAthenaLivingWorldManager::CreateSpawnRequestFromEventRequest : (%hs) The AI spawner system refused the spawn request."), EventData->GetName().c_str());
			continue;
		}

		SpawnRequestIDs.Add(RequestID);
		SpawnRequestPositions[RequestID] = PositionFromProvider;
	}

	const int32 FailedActorCount = EventRequest.ActorCount - SpawnRequestIDs.Num();
	if (EventRuntimeData && FailedActorCount > 0)
	{
		EventRuntimeData->GameSpawnedCount -= FailedActorCount;
	}

	if (SpawnRequestIDs.Num() == 0)
	{
		PointProvider->OnSpawnedActorFailed();
		return;
	}

	FLivingWorldEventInstance EventInstance{};
	EventInstance.Category = EventRequest.Category;
	EventInstance.Event = Event;
	EventInstance.PointProvider = PointProviderActor;
	EventInstance.RespawnTime = -1.f;
	EventInstance.ActorCount = SpawnRequestIDs.Num();
	EventInstances.Add(EventInstance);

	FLivingWorldEventInstance& AddedInstance = EventInstances[EventInstances.Num() - 1];
	for (int32 Index = 0; Index < SpawnRequestIDs.Num(); ++Index)
	{
		AddedInstance.PendingSpawnRequestIDs.Add(SpawnRequestIDs[Index]);
	}

	if (bVerboseLogging)
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("UFortAthenaLivingWorldManager::CreateSpawnRequestFromEventRequest : (%hs) %d spawn request(s) sent to the AI spawner system"), EventData->GetName().c_str(), SpawnRequestIDs.Num());
	}
}

void UFortAthenaLivingWorldManager::OnActorSpawnedCallback(UObject* UserObject, AActor* SpawnedActor, int32 RequestID)
{
	if (UFortAthenaLivingWorldManager* LivingWorldManager = UserObject ? UserObject->Cast<UFortAthenaLivingWorldManager>() : nullptr)
	{
		LivingWorldManager->OnActorSpawned(SpawnedActor, RequestID);
	}
}

void UFortAthenaLivingWorldManager::OnActorSpawned(AActor* Actor, int32 RequestId)
{
	FVector PositionFromProvider = Actor ? Actor->GetActorLocation() : FVector(0.f, 0.f, 0.f);
	const auto PositionIt = SpawnRequestPositions.find(RequestId);
	if (PositionIt != SpawnRequestPositions.end())
	{
		PositionFromProvider = PositionIt->second;
		SpawnRequestPositions.erase(PositionIt);
	}

	for (int32 InstanceIndex = 0; InstanceIndex < EventInstances.Num(); ++InstanceIndex)
	{
		FLivingWorldEventInstance& EventInstance = EventInstances[InstanceIndex];

		int32 PendingIndex = -1;
		for (int32 Index = 0; Index < EventInstance.PendingSpawnRequestIDs.Num(); ++Index)
		{
			if (EventInstance.PendingSpawnRequestIDs[Index] == RequestId)
			{
				PendingIndex = Index;
				break;
			}
		}
		if (PendingIndex == -1)
		{
			continue;
		}

		EventInstance.PendingSpawnRequestIDs.Remove(PendingIndex);

		IFortAthenaLivingWorldPointProviderInterface* PointProvider = EventInstance.PointProvider ? static_cast<IFortAthenaLivingWorldPointProviderInterface*>(EventInstance.PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass())) : nullptr;

		if (Actor && !Actor->IsPendingKillPending())
		{
			EventInstance.ActorInfos.Add(TWeakObjectPtr<AActor>(Actor));
			SpawnedPawns.Add(Actor);

			void (*AddUniqueDynamic)(void*, UObject*, void*, FName) = decltype(AddUniqueDynamic)(ImageBase + 0x3765C88);
			AddUniqueDynamic(&Actor->OnDestroyed, this, nullptr, FName(L"OnSpawnedActorDestroy"));

			if (PointProvider)
			{
				PointProvider->OnSpawnedActor(Actor, PositionFromProvider);
			}

			if (bVerboseLogging)
			{
				UE_LOG(LogLivingWorldManager, Log, TEXT("UFortAthenaLivingWorldManager::OnActorSpawned : %hs spawned for request %d"), Actor->GetName().c_str(), RequestId);
			}
		}
		else
		{
			--EventInstance.ActorCount;
			if (FLivingWorldEventRuntimeData* EventRuntimeData = FindEventRuntimeData(EventInstance.Event))
			{
				--EventRuntimeData->GameSpawnedCount;
			}

			if (PointProvider)
			{
				PointProvider->OnSpawnedActorFailed();
			}

			if (EventInstance.ActorCount <= 0 && EventInstance.ActorInfos.Num() == 0)
			{
				EventInstance.PendingSpawnRequestIDs.Clear();
				EventInstance.ActorInfos.Clear();
				EventInstances.Remove(InstanceIndex);
			}
		}
		return;
	}
}

void UFortAthenaLivingWorldManager::ReleaseInstances()
{
	if (!CachedWorld)
	{
		return;
	}

	const float TimeSeconds = CachedWorld->GetTimeSeconds();
	for (int32 Index = EventInstances.Num() - 1; Index >= 0; --Index)
	{
		FLivingWorldEventInstance& EventInstance = EventInstances[Index];
		if (EventInstance.RespawnTime >= 0.f && TimeSeconds >= EventInstance.RespawnTime)
		{
			EventInstance.PendingSpawnRequestIDs.Clear();
			EventInstance.ActorInfos.Clear();
			EventInstances.Remove(Index);
		}
	}
}

void UFortAthenaLivingWorldManager::RegisterPointProvider(AActor* PointProvider)
{
	AActor* Owner = GetOwner();
	if (!PointProvider || !IsLivingWorldEnabled() || !Owner || Owner->Role != ENetRole::ROLE_Authority)
	{
		return;
	}

	void* InterfaceAddress = PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass());
	if (!InterfaceAddress)
	{
		return;
	}

	TScriptInterface<IFortAthenaLivingWorldPointProviderInterface> PointProviderInterface;
	PointProviderInterface.ObjectPointer = PointProvider;
	PointProviderInterface.InterfacePointer = InterfaceAddress;

	bool bAlreadyRegistered = false;
	for (int32 Index = 0; Index < PointProviders.Num(); ++Index)
	{
		if (PointProviders[Index].ObjectPointer == PointProvider)
		{
			bAlreadyRegistered = true;
			break;
		}
	}
	if (!bAlreadyRegistered)
	{
		PointProviders.Add(PointProviderInterface);
	}

	for (int32 Index = 0; Index < PointProviders.Num(); ++Index)
	{
		UObject* OtherProvider = PointProviders[Index].ObjectPointer;
		AFortAthenaLivingWorldVolume* Volume = OtherProvider && OtherProvider != PointProvider ? OtherProvider->Cast<AFortAthenaLivingWorldVolume>() : nullptr;
		if (Volume)
		{
			Volume->OnPointProviderRegistered(PointProviderInterface);
		}
	}

	FGameplayTagContainer Tags;
	static_cast<IFortAthenaLivingWorldPointProviderInterface*>(InterfaceAddress)->GetFiltersTags(Tags);

	for (auto& Pair : EventRuntimeDataMap)
	{
		FLivingWorldEventRuntimeData& EventRuntimeData = const_cast<FLivingWorldEventRuntimeData&>(Pair.Value());
		if (!EventRuntimeData.ProviderFiltersTagQuery.IsEmpty() && !EventRuntimeData.ProviderFiltersTagQuery.Matches(Tags))
		{
			continue;
		}

		bool bAlreadyMatching = false;
		for (int32 Index = 0; Index < EventRuntimeData.MatchingPointProviders.Num(); ++Index)
		{
			if (EventRuntimeData.MatchingPointProviders[Index].ObjectPointer == PointProvider)
			{
				bAlreadyMatching = true;
				break;
			}
		}
		if (!bAlreadyMatching)
		{
			EventRuntimeData.MatchingPointProviders.Add(PointProviderInterface);
		}
	}

	if (bVerboseLogging)
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("UFortAthenaLivingWorldManager::RegisterPointProvider : %hs registered (%d point providers)"), PointProvider->GetName().c_str(), PointProviders.Num());
	}
}

void UFortAthenaLivingWorldManager::UnregisterPointProvider(AActor* PointProvider)
{
	if (!PointProvider)
	{
		return;
	}

	for (int32 Index = PointProviders.Num() - 1; Index >= 0; --Index)
	{
		if (PointProviders[Index].ObjectPointer == PointProvider)
		{
			PointProviders.Remove(Index);
		}
	}

	for (auto& Pair : EventRuntimeDataMap)
	{
		FLivingWorldEventRuntimeData& EventRuntimeData = const_cast<FLivingWorldEventRuntimeData&>(Pair.Value());
		for (int32 Index = EventRuntimeData.MatchingPointProviders.Num() - 1; Index >= 0; --Index)
		{
			if (EventRuntimeData.MatchingPointProviders[Index].ObjectPointer == PointProvider)
			{
				EventRuntimeData.MatchingPointProviders.Remove(Index);
			}
		}
	}

	for (int32 Index = 0; Index < PendingEventRequests.Num(); ++Index)
	{
		if (PendingEventRequests[Index].PointProvider == PointProvider)
		{
			PendingEventRequests[Index].PointProvider = nullptr;
		}
	}

	for (int32 Index = 0; Index < EventInstances.Num(); ++Index)
	{
		if (EventInstances[Index].PointProvider == PointProvider)
		{
			EventInstances[Index].PointProvider = nullptr;
		}
	}
}

void UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(AActor* PointProvider)
{
	if (UFortAthenaLivingWorldManager* LivingWorldManager = GetLivingWorldManager(PointProvider))
	{
		LivingWorldManager->RegisterPointProvider(PointProvider);
	}
}

void UFortAthenaLivingWorldManager::LivingWorldManagerUnregisterPointProvider(AActor* PointProvider)
{
	if (UFortAthenaLivingWorldManager* LivingWorldManager = GetLivingWorldManager(PointProvider))
	{
		LivingWorldManager->UnregisterPointProvider(PointProvider);
	}
}

DEFINE_FUNCTION(UFortAthenaLivingWorldManager::execLivingWorldManagerRegisterPointProvider)
{
	P_GET_OBJECT(AActor,Z_Param_PointProvider);
	P_FINISH;
	UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(Z_Param_PointProvider);
}

DEFINE_FUNCTION(UFortAthenaLivingWorldManager::execLivingWorldManagerUnregisterPointProvider)
{
	P_GET_OBJECT(AActor,Z_Param_PointProvider);
	P_FINISH;
	UFortAthenaLivingWorldManager::LivingWorldManagerUnregisterPointProvider(Z_Param_PointProvider);
}

void UFortAthenaLivingWorldManager::HandleCurrentPlaylistLoaded(AFortGameStateAthena* GameState, FName PlaylistName)
{
	if (!GameState)
	{
		return;
	}

	UFortAthenaLivingWorldManager* LivingWorldManager = GameState->FindComponentByClass<UFortAthenaLivingWorldManager>();
	if (!LivingWorldManager)
	{
		UE_LOG(LogLivingWorldManager, Warning, TEXT("UFortAthenaLivingWorldManager::HandleCurrentPlaylistLoaded : %hs has no living world manager component"), GameState->GetName().c_str());
		return;
	}

	const float LagerEnabledValue = LivingWorldManager->LagerEnabled.GetValueAtLevel(0.f);
	if (LagerEnabledValue <= 0.f)
	{
		UE_LOG(LogLivingWorldManager, Log, TEXT("UFortAthenaLivingWorldManager::HandleCurrentPlaylistLoaded : LagerEnabled evaluates to %f from the shipped curve table, applying the live hotfix value 1"), LagerEnabledValue);
		LivingWorldManager->LagerEnabled.Value = 1.f;
		LivingWorldManager->LagerEnabled.Curve.CurveTable = nullptr;
		LivingWorldManager->LagerEnabled.Curve.RowName = FName();
	}

	LivingWorldManager->bVerboseLogging = true;

	const FGameplayTagContainer PlaylistContextTags = GameState->GetAthenaPlaylistContextTags();
	if (!LivingWorldManager->CachedConfig)
	{
		LivingWorldManager->OnCurrentPlaylistLoaded(PlaylistName, PlaylistContextTags);
	}

	AFortAthenaLivingWorldVolume::OnCurrentPlaylistLoadedForAllVolumes(GameState->GetWorld(), PlaylistName, PlaylistContextTags);
}

void UFortAthenaLivingWorldManager::Init()
{
	Memory::HookDetour(ImageBase + 0x3C7B468, InitializeHook, &InitializeOG);
	Memory::SwapVTableEntryInAllSubClasses<UFortAthenaLivingWorldManager>(106, TickComponentHook);

	UClass* ManagerClass = UFortAthenaLivingWorldManager::StaticClass();
	Memory::HookUFunction(ManagerClass->GetFunction("FortAthenaLivingWorldManager", "LivingWorldManagerRegisterPointProvider"), (void*)&UFortAthenaLivingWorldManager::execLivingWorldManagerRegisterPointProvider);
	Memory::HookUFunction(ManagerClass->GetFunction("FortAthenaLivingWorldManager", "LivingWorldManagerUnregisterPointProvider"), (void*)&UFortAthenaLivingWorldManager::execLivingWorldManagerUnregisterPointProvider);
}

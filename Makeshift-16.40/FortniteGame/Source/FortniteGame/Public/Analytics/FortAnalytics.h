#include "pch.h"

struct FAthenaSafeZoneTelemetryInfo
{
	FVector Center;
	float Radius;
	uint8 Phase;
	EFortSafeZoneState State;

	FAthenaSafeZoneTelemetryInfo(const FVector& InCenter, float InRadius, uint8 InPhase, EFortSafeZoneState InState)
		: Center(InCenter)
		, Radius(InRadius)
		, Phase(InPhase)
		, State(InState)
	{
	}
};
static_assert(offsetof(FAthenaSafeZoneTelemetryInfo, Radius) == 0x00000C, "Member 'FAthenaSafeZoneTelemetryInfo::Radius' has a wrong offset!");
static_assert(offsetof(FAthenaSafeZoneTelemetryInfo, Phase) == 0x000010, "Member 'FAthenaSafeZoneTelemetryInfo::Phase' has a wrong offset!");
static_assert(offsetof(FAthenaSafeZoneTelemetryInfo, State) == 0x000011, "Member 'FAthenaSafeZoneTelemetryInfo::State' has a wrong offset!");

class UFortAnalytics
{
public:
	static void FireEvent_AthenaNewSafeZone(AFortGameModeAthena* GameMode, const FAthenaSafeZoneTelemetryInfo& NewSafeZone, const FAthenaSafeZoneTelemetryInfo& PreviousSafeZone)
	{
		void (*Fn)(AFortGameModeAthena*, const FAthenaSafeZoneTelemetryInfo*, const FAthenaSafeZoneTelemetryInfo*) = decltype(Fn)(ImageBase + 0x43F5DE8);
		Fn(GameMode, &NewSafeZone, &PreviousSafeZone);
	}
};

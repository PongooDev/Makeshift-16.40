#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

int32 UFortKismetLibrary::IncrementAnalyticMatchCount(const UObject* WorldContextObject, const EAnalyticMatchCounts MatchCountID, const int32 AmountToAdd) {
	static const UWorld* AnalyticMatchCountsWorld = nullptr;
	static int32 AnalyticMatchCounts[static_cast<int32>(EAnalyticMatchCounts::Count)] = {};

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || MatchCountID >= EAnalyticMatchCounts::Count) {
		return 0;
	}

	if (AnalyticMatchCountsWorld != World) {
		AnalyticMatchCountsWorld = World;
		FMemory::Memzero(AnalyticMatchCounts, sizeof(AnalyticMatchCounts));
	}

	AnalyticMatchCounts[static_cast<int32>(MatchCountID)] += AmountToAdd;
	return AnalyticMatchCounts[static_cast<int32>(MatchCountID)];
}

DEFINE_FUNCTION(UFortKismetLibrary::execIncrementAnalyticMatchCount)
{
	P_GET_OBJECT(UObject,Z_Param_WorldContextObject);
	P_GET_ENUM(EAnalyticMatchCounts,Z_Param_MatchCountID);
	P_GET_PROPERTY(FIntProperty,Z_Param_AmountToAdd);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(int32*)Z_Param__Result=UFortKismetLibrary::IncrementAnalyticMatchCount(Z_Param_WorldContextObject,EAnalyticMatchCounts(Z_Param_MatchCountID),Z_Param_AmountToAdd);
	P_NATIVE_END;
}

AFortPlayerController* UFortKismetLibrary::GetFortPlayerControllerFromActor(AActor* Actor) {
	AFortPlayerController* (*Fn)(AActor*) = decltype(Fn)(InSDKUtils::GetImageBase() + 0x1F9B498);
	return Fn(Actor);
}

void UFortKismetLibrary::ChangeTeam(AActor* PlayerToSwitch, AActor* Instigator, uint8 NewTeam, const FGameplayTagContainer& ChangeTeamTags) {
	AFortPlayerController* PlayerController = GetFortPlayerControllerFromActor(PlayerToSwitch);
	AFortPlayerControllerAthena* PlayerControllerAthena = PlayerController ? PlayerController->Cast<AFortPlayerControllerAthena>() : nullptr;
	if (!PlayerControllerAthena) {
		return;
	}

	AFortPlayerStateAthena* PlayerState = PlayerControllerAthena->PlayerState ? PlayerControllerAthena->PlayerState->Cast<AFortPlayerStateAthena>() : nullptr;
	if (PlayerState) {
		PlayerState->ChangeTeamInfo.Instigator = Instigator;
		PlayerState->ChangeTeamInfo.ChangeTeamTags = ChangeTeamTags;
	}

	PlayerControllerAthena->ServerSetTeam_Implementation(NewTeam);
}

DEFINE_FUNCTION(UFortKismetLibrary::execChangeTeam)
{
	P_GET_OBJECT(AActor,Z_Param_PlayerToSwitch);
	P_GET_OBJECT(AActor,Z_Param_Instigator);
	P_GET_PROPERTY(FByteProperty,Z_Param_NewTeam);
	P_GET_STRUCT_REF(FGameplayTagContainer,Z_Param_Out_ChangeTeamTags);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortKismetLibrary::ChangeTeam(Z_Param_PlayerToSwitch,Z_Param_Instigator,Z_Param_NewTeam,Z_Param_Out_ChangeTeamTags);
	P_NATIVE_END;
}

void UFortKismetLibrary::Init() {
	Memory::HookDetour(ImageBase + 0x5098E58, execIncrementAnalyticMatchCount, nullptr);
	Memory::HookDetour(ImageBase + 0x508ADCC, execChangeTeam, nullptr);
}

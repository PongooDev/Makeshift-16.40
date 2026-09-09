#include "pch.h"

void AFortPlayerStateAthena::SetKillScore(int32 NewScore) {
	UE_LOG(LogFort, Log, TEXT("FORT-377179 AFortPlayerStateAthena::SetKillScore(%d) (OldKillScore = %d)"), NewScore, KillScore);
	KillScore = NewScore;
}

void AFortPlayerStateAthena::SetTeamKillScore(int32 NewScore) {
	TeamKillScore = NewScore;
}

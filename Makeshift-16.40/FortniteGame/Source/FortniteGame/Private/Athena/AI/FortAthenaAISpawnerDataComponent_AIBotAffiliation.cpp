#include "pch.h"

void UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawnedHook(UFortAthenaAISpawnerDataComponent_AIBotAffiliation* This, APawn* PawnAI)
{
	This->UFortAthenaAISpawnerDataComponent_AffiliationBase::OnSpawned(PawnAI);

	AController* Controller = PawnAI->Controller;
	AFortAthenaAIBotController* BotController = Controller ? Controller->Cast<AFortAthenaAIBotController>() : nullptr;
	if (BotController)
	{
		if (This->bOverrideIsAnAthenaGameParticipant)
		{
			BotController->bIsAnAthenaGameParticipant = This->bIsAnAthenaGameParticipant;
		}

		UFortServerBotManagerAthena* BotManager = BotController->CachedBotManager;
		if (BotManager)
		{
			int32 SquadID = 0;
			if (This->GetSquadID(&SquadID))
			{
				const uint8 SquadID8 = static_cast<uint8>(SquadID);
				UE_LOG(LogAISpawnerData, Verbose, TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned Assigning Affiliation to Pawn=%hs Controller=%hs with CustomSquadId=%d"), PawnAI->GetName().c_str(), Controller->GetName().c_str(), SquadID);
				BotManager->AssignTeamAndSquad(BotController, &SquadID8);
			}
			else
			{
				UE_LOG(LogAISpawnerData, Verbose, TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned Assigning Affiliation to Pawn=%hs Controller=%hs"), PawnAI->GetName().c_str(), Controller->GetName().c_str());
				BotManager->AssignTeamAndSquad(BotController, nullptr);
			}
		}
		else
		{
			UE_LOG(LogAISpawnerData, Warning, TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned Fail to set Affiliation for Pawn=%hs Controller=%hs as no UFortServerBotManagerAthena found!"), PawnAI->GetName().c_str(), Controller->GetName().c_str());
		}
	}
	else
	{
		UE_LOG(LogAISpawnerData, Warning, TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned Can't set team Pawn=%hs Controller=%hs as can't retrieve AIBotController on pawn!"), PawnAI->GetName().c_str(), Controller ? Controller->GetName().c_str() : "");
	}
}

void UFortAthenaAISpawnerDataComponent_AIBotAffiliation::Init()
{
	Memory::HookDetour(ImageBase + 0x44CA084, OnSpawnedHook);
}

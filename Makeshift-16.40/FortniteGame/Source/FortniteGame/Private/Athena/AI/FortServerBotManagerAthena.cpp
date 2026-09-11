#include "pch.h"

void UFortServerBotManagerAthena::CheckForBotBrainActivation()
{
	if (!CachedAIPopulationTracker)
	{
		UE_LOG(LogAthenaBots, Error, TEXT("No CachedAIPopulationTracker present!"));
		return;
	}

	bCanActivateBotsBrain = true;

	for (int32 Index = 0; Index < CachedAIPopulationTracker->PlayerBots.Num(); ++Index)
	{
		AFortAthenaAIBotController* BotController = CachedAIPopulationTracker->PlayerBots[Index].BotController;
		if (BotController)
		{
			BotController->RunBehaviorTree(BotController->BehaviorTree);
		}
	}

	for (int32 Index = 0; Index < CachedAIPopulationTracker->NoneAthenaParticipantBots.Num(); ++Index)
	{
		AFortAthenaAIBotController* BotController = CachedAIPopulationTracker->NoneAthenaParticipantBots[Index].BotController;
		if (BotController)
		{
			BotController->RunBehaviorTree(BotController->BehaviorTree);
		}
	}
}

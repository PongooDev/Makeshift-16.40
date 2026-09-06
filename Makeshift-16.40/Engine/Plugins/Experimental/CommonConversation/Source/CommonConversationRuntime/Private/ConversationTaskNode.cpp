// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

FConversationTaskResult UConversationTaskNode::ExecuteTaskNodeWithSideEffects(const FConversationContext& InContext) const
{
	ensure(InContext.GetTaskBeingConsidered() == this);

	TGuardValue<UObject*> Swapper(EvalWorldContextObj, InContext.GetWorld());

	FConversationTaskResult Result;

	if (InContext.IsServerContext())
	{
		Result = ExecuteTaskNode(InContext);
		ensureMsgf(Result.GetType() != EConversationTaskResultType::Invalid, TEXT("Conversation Node %hs - Returned an Invalid result indicating no specific decision was made on how to continue."), GetName().c_str());

		// After executing the task we need to determine if we should run side effects on the server and client.
		if (Result.CanConversationContinue())
		{
			for (UConversationSubNode* SubNode : SubNodes)
			{
				if (UConversationSideEffectNode* SideEffectNode = SubNode ? SubNode->Cast<UConversationSideEffectNode>() : nullptr)
				{
					UE_LOG(LogCommonConversationRuntime, Verbose, TEXT("\tRunning side effect %s"), *GetPathNameSafe(SideEffectNode));
					SideEffectNode->CauseSideEffect(InContext);
				}
			}

			FConversationParticipants Participants = InContext.GetParticipantsCopy();
			for (const FConversationParticipantEntry& ParticipantEntry : Participants.List)
			{
				if (UConversationParticipantComponent* Component = ParticipantEntry.GetParticipantComponent())
				{
					// Notify each client in the conversation
					if (Component->GetOwner()->RemoteRole == ENetRole::ROLE_AutonomousProxy)
					{
						Component->ServerNotifyExecuteTaskAndSideEffects(InContext.GetCurrentNodeHandle());
					}
				}
			}
		}
	}

	if (InContext.IsClientContext())
	{
		ExecuteClientEffects(InContext);

		for (UConversationSubNode* SubNode : SubNodes)
		{
			if (UConversationSideEffectNode* SideEffect = SubNode ? SubNode->Cast<UConversationSideEffectNode>() : nullptr)
			{
				SideEffect->CauseSideEffect(InContext);
			}
		}
	}

	return Result;
}

void UConversationTaskNode::GenerateChoicesForDestinations(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext, const TArray<FGuid>& CandidateDestinations)
{
	check(InContext.IsServerContext());

	UWorld* World = InContext.GetWorld();

	for (const FGuid& DestinationGUID : CandidateDestinations)
	{
		UConversationNode* DestinationNode = InContext.GetConversationRegistry().GetRuntimeNodeFromGUID(DestinationGUID);
		if (UConversationTaskNode* DestinationTaskNode = DestinationNode ? DestinationNode->Cast<UConversationTaskNode>() : nullptr)
		{
			TGuardValue<UObject*> Swapper(DestinationTaskNode->EvalWorldContextObj, World);

			FConversationContext DestinationContext = InContext.CreateChildContext(DestinationTaskNode);

			const int32 StartingNumber = BranchBuilder.Num();

			DestinationTaskNode->GatherChoices(BranchBuilder, DestinationContext);

			// If a node has no choices, but we're generating the choices, we need to have this node as 'a' choice, even if
			// it's not something we're ever sending to the client, we just need to know this is a valid path for the
			// conversation to flow.
			if (BranchBuilder.Num() == StartingNumber)
			{
				const EConversationRequirementResult RequirementResult = DestinationTaskNode->CheckRequirements(InContext);

				if (RequirementResult == EConversationRequirementResult::Passed)
				{
					FClientConversationOptionEntry DefaultChoice;
					DefaultChoice.ChoiceReference.NodeReference = DestinationGUID;
					DefaultChoice.ChoiceType = EConversationChoiceType::ServerOnly;
					BranchBuilder.AddChoice(DestinationContext, std::move(DefaultChoice));
				}
			}
		}
	}
}

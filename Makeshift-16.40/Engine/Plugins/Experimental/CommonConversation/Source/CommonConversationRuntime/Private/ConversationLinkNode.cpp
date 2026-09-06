// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

void UConversationLinkNode::GatherChoicesHook(const UConversationLinkNode* This, FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& Context)
{
	TArray<FGuid> PotentialStartingPoints = Context.GetConversationRegistry().GetOutputLinkGUIDs(This->RemoteEntryTag);

	if (PotentialStartingPoints.Num() > 0)
	{
		TArray<FGuid> LegalStartingPoints = Context.GetActiveConversation()->DetermineBranches(PotentialStartingPoints, EConversationRequirementResult::FailedButVisible);
		FConversationContext ReturnScopeContext = Context.CreateReturnScopeContext(This->GetNodeGuid());
		UConversationTaskNode::GenerateChoicesForDestinations(BranchBuilder, ReturnScopeContext, LegalStartingPoints);
	}
}

void UConversationLinkNode::Init() {
	Memory::SwapVTableEntryInAllSubClasses<UConversationLinkNode>(85, GatherChoicesHook);
}

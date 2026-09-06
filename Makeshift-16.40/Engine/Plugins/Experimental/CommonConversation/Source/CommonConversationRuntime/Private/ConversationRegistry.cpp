// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

//======================================================================================

const UConversationNode* FConversationNodeHandle::TryToResolve_Slow(UWorld* InWorld) const
{
	UConversationRegistry* Registry = UConversationRegistry::GetFromWorld(InWorld);
	return Registry->GetRuntimeNodeFromGUID(NodeGUID);
}

const UConversationNode* FConversationNodeHandle::TryToResolve(const FConversationContext& Context) const
{
	return Context.GetConversationRegistry().GetRuntimeNodeFromGUID(NodeGUID);
}

TArray<FGuid> UConversationRegistry::GetEntryPointGUIDs(FGameplayTag EntryPoint) const
{
	const_cast<UConversationRegistry*>(this)->BuildDependenciesGraph();

	for (int32 Index = 0; Index < EntryTagToEntryList.NumAllocated(); ++Index)
	{
		if (EntryTagToEntryList.IsValidIndex(Index) && EntryTagToEntryList[Index].Key() == EntryPoint)
		{
			return EntryTagToEntryList[Index].Value();
		}
	}

	return TArray<FGuid>();
}

TArray<FGuid> UConversationRegistry::GetOutputLinkGUIDs(FGameplayTag EntryPoint) const
{
	TArray<FGuid> SourceGUIDs = GetEntryPointGUIDs(EntryPoint);
	return GetOutputLinkGUIDs(SourceGUIDs);
}

TArray<FGuid> UConversationRegistry::GetOutputLinkGUIDs(const FGuid& SourceGUID) const
{
	TArray<FGuid> SourceGUIDs;
	SourceGUIDs.Add(SourceGUID);
	return GetOutputLinkGUIDs(SourceGUIDs);
}

TArray<FGuid> UConversationRegistry::GetOutputLinkGUIDs(const TArray<FGuid>& SourceGUIDs) const
{
	const_cast<UConversationRegistry*>(this)->BuildDependenciesGraph();

	TArray<FGuid> Result;

	for (const FGuid& SourceGUID : SourceGUIDs)
	{
		if (const UConversationDatabase* SourceConversation = GetConversationFromNodeGUID(SourceGUID))
		{
			UConversationNode* SourceNode = nullptr;
			for (int32 Index = 0; Index < SourceConversation->ReachableNodeMap.NumAllocated(); ++Index)
			{
				if (SourceConversation->ReachableNodeMap.IsValidIndex(Index) && SourceConversation->ReachableNodeMap[Index].Key() == SourceGUID)
				{
					SourceNode = SourceConversation->ReachableNodeMap[Index].Value();
					break;
				}
			}
			if (ensure(SourceNode))
			{
				if (UConversationNodeWithLinks* SourceNodeWithLinks = SourceNode->Cast<UConversationNodeWithLinks>())
				{
					for (const FGuid& OutputConnection : SourceNodeWithLinks->OutputConnections)
					{
						Result.Add(OutputConnection);
					}
				}
			}
		}
	}

	return Result;
}

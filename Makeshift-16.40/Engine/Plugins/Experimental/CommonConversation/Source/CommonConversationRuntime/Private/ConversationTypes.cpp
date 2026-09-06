// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

//////////////////////////////////////////////////////////////////////

static FString LexToString(const TArray<FConversationNodeParameterPair>& InParameters)
{
	if (InParameters.Num() > 0)
	{
		FString MapString;
		MapString += TEXT("{ ");
		bool bFirst = true;
		for (const FConversationNodeParameterPair& Entry : InParameters)
		{
			if (!bFirst)
			{
				MapString += TEXT(", ");
			}

			bFirst = false;

			MapString += Entry.Name;
			MapString += TEXT(": ");
			MapString += Entry.Value;
		}
		MapString += TEXT("}");

		return MapString;
	}

	return FString();
}

const FConversationChoiceReference FConversationChoiceReference::Empty = FConversationChoiceReference();

FString FConversationChoiceReference::ToString() const
{
	return FString::Printf(TEXT("(Node=%s, Params=%s)"), *NodeReference.ToString(), *LexToString(NodeParameters));
}

//////////////////////////////////////////////////////////////////////

const FAdvanceConversationRequest FAdvanceConversationRequest::Any = FAdvanceConversationRequest();

FString FAdvanceConversationRequest::ToString() const
{
	return FString::Printf(TEXT("(Choice=%s, UserParams=%s)"), *Choice.ToString(), *LexToString(UserParameters));
}

//////////////////////////////////////////////////////////////////////

FAdvanceConversationRequest FClientConversationOptionEntry::ToAdvanceConversationRequest(const TArray<FConversationNodeParameterPair>& InUserParameters) const
{
	FAdvanceConversationRequest Request;
	Request.Choice = ChoiceReference;
	Request.UserParameters = InUserParameters;

	return Request;
}

//////////////////////////////////////////////////////////////////////

void FConversationBranchPointBuilder::AddChoice(const FConversationContext& InContext, FClientConversationOptionEntry&& InChoice)
{
	void (*Fn)(FConversationBranchPointBuilder*, const FConversationContext*, FClientConversationOptionEntry*) = decltype(Fn)(ImageBase + 0x376000C);
	Fn(this, &InContext, &InChoice);
}

//////////////////////////////////////////////////////////////////////

const FConversationParticipantEntry* FConversationParticipants::GetParticipant(FGameplayTag ParticipantID) const
{
	const FConversationParticipantEntry* (*Fn)(const TArray<FConversationParticipantEntry>*, FGameplayTag) = decltype(Fn)(ImageBase + 0x37610FC);
	return Fn(&List, ParticipantID);
}

UConversationParticipantComponent* FConversationParticipants::GetParticipantComponent(FGameplayTag ParticipantID) const
{
	if (const FConversationParticipantEntry* Entry = GetParticipant(ParticipantID))
	{
		return Entry->GetParticipantComponent();
	}
	return nullptr;
}

bool FConversationParticipants::Contains(AActor* PotentialParticipant) const
{
	for (const FConversationParticipantEntry& Entry : List)
	{
		if (Entry.Actor == PotentialParticipant)
		{
			return true;
		}
	}

	return false;
}

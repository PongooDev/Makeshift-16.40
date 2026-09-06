// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

//@TODO: CONVERSATION: Assert or otherwise guard all the Server* functions to only execute on the authority

void UConversationParticipantComponent::ServerNotifyConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant)
{
	AActor* Owner = GetOwner();
	if (Owner->Role == ENetRole::ROLE_Authority)
	{
		check(Conversation);
		Auth_CurrentConversation = Conversation;
		Auth_Conversations.Add(Conversation);

		//@TODO: CONVERSATION: ClientUpdateParticipants, we need to do this immediately so when we tell the client a task has been
		// executed, the client has knowledge of what participants, before any client side task effects need to execute.
		ClientUpdateParticipants(Auth_CurrentConversation->GetParticipantsCopy());

		if (ConversationsActive == 0)
		{
			OnEnterConversationState();
		}

		ConversationsActive++;

		ClientStartConversation(Conversation, AsParticipant);

		if (Owner->RemoteRole == ENetRole::ROLE_AutonomousProxy)
		{
			ClientUpdateConversations(ConversationsActive);
		}
	}
}

void UConversationParticipantComponent::ServerNotifyConversationEnded(UConversationInstance* Conversation)
{
	AActor* Owner = GetOwner();
	if (Owner->Role == ENetRole::ROLE_Authority)
	{
		if (Auth_CurrentConversation == Conversation)
		{
			check(Conversation);
			Auth_CurrentConversation = nullptr;
			const int32 ConversationIndex = Auth_Conversations.Find(Conversation);
			if (ConversationIndex != INDEX_NONE)
			{
				Auth_Conversations.RemoveAt(ConversationIndex);
			}

			ConversationsActive--;

			if (Owner->RemoteRole == ENetRole::ROLE_AutonomousProxy)
			{
				ClientUpdateConversations(ConversationsActive);
			}

			if (ConversationsActive == 0)
			{
				OnLeaveConversationState();
			}
		}
	}
}

void UConversationParticipantComponent::ServerNotifyExecuteTaskAndSideEffects(const FConversationNodeHandle& Handle)
{
	if (GetOwner()->Role == ENetRole::ROLE_Authority)
	{
		ClientExecuteTaskAndSideEffects(Handle);
	}
}

void UConversationParticipantComponent::ServerForAllConversationsRefreshChoices(UConversationInstance* IgnoreConversation)
{
	if (GetOwner()->Role == ENetRole::ROLE_Authority)
	{
		for (UConversationInstance* Conversation : Auth_Conversations)
		{
			if (Conversation == IgnoreConversation)
			{
				continue;
			}

			Conversation->ServerRefreshConversationChoices();
		}
	}
}

FConversationNodeHandle UConversationParticipantComponent::GetCurrentNodeHandle() const
{
	if (GetOwner()->Role == ENetRole::ROLE_Authority)
	{
		if (Auth_CurrentConversation)
		{
			return Auth_CurrentConversation->GetCurrentNodeHandle();
		}
	}
	else
	{
		return LastMessage.CurrentNode;
	}

	return FConversationNodeHandle();
}

const FConversationParticipantEntry* UConversationParticipantComponent::GetParticipant(const FGameplayTag& ParticipantTag) const
{
	if (GetOwner()->Role == ENetRole::ROLE_Authority)
	{
		if (Auth_CurrentConversation)
		{
			return Auth_CurrentConversation->GetParticipant(ParticipantTag);
		}
	}
	else
	{
		return LastMessage.Participants.GetParticipant(ParticipantTag);
	}

	return nullptr;
}

void UConversationParticipantComponent::SendClientConversationMessage(const FConversationContext& Context, const FClientConversationMessagePayload& Payload)
{
	LastMessage = Payload;

	//@TODO: CONVERSATION: We could potentially send the user no choices?  I guess that's a possibility.
	if (GetOwner()->RemoteRole == ENetRole::ROLE_AutonomousProxy)
	{
		ClientUpdateConversation(LastMessage);
	}
}

void UConversationParticipantComponent::SendClientUpdatedChoices(const FConversationContext& Context)
{
	if (GetOwner()->RemoteRole == ENetRole::ROLE_AutonomousProxy)
	{
		const TArray<FClientConversationOptionEntry> NewOptions = Context.GetActiveConversation()->GetCurrentUserConversationChoices();
		if (!ArraysEqual(NewOptions, LastMessage.Options))
		{
			LastMessage.Options = NewOptions;
			ClientUpdateConversation(LastMessage);
		}
	}
}

bool UConversationParticipantComponent::IsInActiveConversation() const
{
	if (Auth_CurrentConversation)
	{
		return true;
	}

	return ConversationsActive > 0;
}

void UConversationParticipantComponent::ServerAdvanceConversation_Implementation(const FAdvanceConversationRequest& InChoicePicked)
{
	if (GetOwner()->Role == ENetRole::ROLE_Authority)
	{
		if (Auth_CurrentConversation)
		{
			Auth_CurrentConversation->ServerAdvanceConversation(InChoicePicked);
		}
	}
}

void UConversationParticipantComponent::ServerAbortAllConversations()
{
	TArray<UConversationInstance*> Conversations = Auth_Conversations;
	for(UConversationInstance* Conversation : Conversations)
	{
		Conversation->ServerAbortConversation();
	}
}

void UConversationParticipantComponent::ServerAdvanceConversationHook(UConversationParticipantComponent* This, const FAdvanceConversationRequest& InChoicePicked)
{
	This->ServerAdvanceConversation_Implementation(InChoicePicked);
}

void UConversationParticipantComponent::Init() {
	Memory::SwapVTableEntryInAllSubClasses<UConversationParticipantComponent>(134, ServerAdvanceConversationHook);
}

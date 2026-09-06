// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

FConversationContext FConversationContext::CreateServerContext(UConversationInstance* InActiveConversation, const UConversationTaskNode* InTaskBeingConsidered)
{
	FConversationContext Context;
	Context.ActiveConversation = InActiveConversation;
	Context.TaskBeingConsidered = InTaskBeingConsidered;
	Context.bServer = true;
	Context.ConversationRegistry = UConversationRegistry::GetFromWorld(InActiveConversation->GetWorld());

	return Context;
}

FConversationContext FConversationContext::CreateClientContext(UConversationParticipantComponent* InParticipantComponent, const UConversationTaskNode* InTaskBeingConsidered)
{
	FConversationContext Context;
	FConversationContext* (*Fn)(FConversationContext*, UConversationParticipantComponent*, const UConversationTaskNode*) = decltype(Fn)(ImageBase + 0x3760884);
	Fn(&Context, InParticipantComponent, InTaskBeingConsidered);

	return Context;
}

FConversationContext FConversationContext::CreateChildContext(const UConversationTaskNode* NewTaskBeingConsidered) const
{
	FConversationContext Context = *this;
	Context.TaskBeingConsidered = NewTaskBeingConsidered;

	return Context;
}

FConversationContext FConversationContext::CreateReturnScopeContext(const FConversationNodeHandle& NewReturnScope) const
{
	FConversationContext Context = *this;
	Context.ReturnScopeStack.Add(NewReturnScope);

	return Context;
}

UWorld* FConversationContext::GetWorld() const
{
	if (bServer)
	{
		if (ActiveConversation)
		{
			return ActiveConversation->GetWorld();
		}
	}
	else
	{
		if (ensure(ClientParticipant))
		{
			return ClientParticipant->GetWorld();
		}
	}

	return nullptr;
}

FConversationNodeHandle FConversationContext::GetCurrentNodeHandle() const
{
	if (bServer)
	{
		if (ActiveConversation)
		{
			return ActiveConversation->GetCurrentNodeHandle();
		}
	}
	else
	{
		if (ensure(ClientParticipant))
		{
			return ClientParticipant->GetCurrentNodeHandle();
		}
	}

	return FConversationNodeHandle();
}

FConversationParticipants FConversationContext::GetParticipantsCopy() const
{
	if (bServer)
	{
		if (ActiveConversation)
		{
			return ActiveConversation->GetParticipantsCopy();
		}
	}
	else
	{
		if (ensure(ClientParticipant))
		{
			return ClientParticipant->LastMessage.Participants;
		}
	}

	return FConversationParticipants();
}

const FConversationParticipantEntry* FConversationContext::GetParticipant(const FGameplayTag& ParticipantTag) const
{
	const FConversationParticipantEntry* (*Fn)(const FConversationContext*, const FGameplayTag*) = decltype(Fn)(ImageBase + 0x376108C);
	return Fn(this, &ParticipantTag);
}

UConversationParticipantComponent* FConversationContext::GetParticipantComponent(const FGameplayTag& ParticipantTag) const
{
	if (const FConversationParticipantEntry* Participant = GetParticipant(ParticipantTag))
	{
		return Participant->GetParticipantComponent();
	}

	return nullptr;
}

AActor* FConversationContext::GetParticipantActor(const FGameplayTag& ParticipantTag) const
{
	if (const FConversationParticipantEntry* Participant = GetParticipant(ParticipantTag))
	{
		return Participant->Actor;
	}

	return nullptr;
}

UConversationRegistry& FConversationContext::GetConversationRegistry() const { check(ConversationRegistry) return *ConversationRegistry; }

UConversationInstance* FConversationContext::GetActiveConversation() const { ensure(bServer); return ActiveConversation; }

const FAdvanceConversationRequest& FConversationTaskResult::GetChoice() const { ensure(Type == EConversationTaskResultType::AdvanceConversationWithChoice); return AdvanceToChoice; }
const FClientConversationMessage& FConversationTaskResult::GetMessage() const { ensure(Type == EConversationTaskResultType::PauseConversationAndSendClientChoices); return Message; }

//////////////////////////////////////////////////////////////////////

UConversationInstance* UConversationContextHelpers::GetConversationInstance(const FConversationContext& Context)
{
	if (UConversationInstance* Conversation = Context.GetActiveConversation())
	{
		return Conversation;
	}

	UE_LOG(LogCommonConversationRuntime, Error, TEXT("Bad conversation context"));
	return nullptr;
}

UConversationParticipantComponent* UConversationContextHelpers::GetConversationParticipant(const FConversationContext& Context, const FGameplayTag& ParticipantTag)
{
	return Context.GetParticipantComponent(ParticipantTag);
}

AActor* UConversationContextHelpers::GetConversationParticipantActor(const FConversationContext& Context, const FGameplayTag& ParticipantTag)
{
	return Context.GetParticipantActor(ParticipantTag);
}

FConversationNodeHandle UConversationContextHelpers::GetCurrentConversationNodeHandle(const FConversationContext& Context)
{
	if (UConversationInstance* Conversation = GetConversationInstance(Context))
	{
		return Conversation->GetCurrentNodeHandle();
	}

	return FConversationNodeHandle();
}

void UConversationContextHelpers::MakeConversationParticipant(const FConversationContext& Context, AActor* ParticipantActor, const FGameplayTag& ParticipantTag)
{
	if (ParticipantActor == nullptr)
	{
		UE_LOG(LogCommonConversationRuntime, Error, TEXT("MakeConversationParticipant needs a valid participant actor"));
		return;
	}

	if (!ParticipantTag.IsValid())
	{
		UE_LOG(LogCommonConversationRuntime, Error, TEXT("MakeConversationParticipant needs a valid participant tag"));
		return;
	}

	if (UConversationInstance* Conversation = GetConversationInstance(Context))
	{
		Conversation->ServerAssignParticipant(ParticipantTag, ParticipantActor);
	}
}

FConversationTaskResult UConversationContextHelpers::AdvanceConversation(const FConversationContext& Context)
{
	return FConversationTaskResult::AdvanceConversation();
}

FConversationTaskResult UConversationContextHelpers::AdvanceConversationWithChoice(const FConversationContext& Context, const FAdvanceConversationRequest& InPickedChoice)
{
	return FConversationTaskResult::AdvanceConversationWithChoice(InPickedChoice);
}

FConversationTaskResult UConversationContextHelpers::PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& Message)
{
	return FConversationTaskResult::PauseConversationAndSendClientChoices(Message);
}

FConversationTaskResult UConversationContextHelpers::ReturnToLastClientChoice(const FConversationContext& Context)
{
	return FConversationTaskResult::ReturnToLastClientChoice();
}

FConversationTaskResult UConversationContextHelpers::ReturnToCurrentClientChoice(const FConversationContext& Context)
{
	return FConversationTaskResult::ReturnToCurrentClientChoice();
}

FConversationTaskResult UConversationContextHelpers::ReturnToConversationStart(const FConversationContext& Context)
{
	return FConversationTaskResult::ReturnToConversationStart();
}

FConversationTaskResult UConversationContextHelpers::AbortConversation(const FConversationContext& Context)
{
	return FConversationTaskResult::AbortConversation();
}

bool UConversationContextHelpers::CanConversationContinue(const FConversationTaskResult& ConversationTasResult)
{
	return ConversationTasResult.CanConversationContinue();
}

UConversationParticipantComponent* UConversationContextHelpers::FindConversationComponent(AActor* Actor)
{
	if (Actor != nullptr)
	{
		return Actor->FindComponentByClass<UConversationParticipantComponent>();
	}

	return nullptr;
}

DEFINE_FUNCTION(UConversationContextHelpers::execMakeConversationParticipant)
{
	P_GET_STRUCT_REF(FConversationContext,Z_Param_Out_Context);
	P_GET_OBJECT(AActor,Z_Param_ParticipantActor);
	P_GET_STRUCT(FGameplayTag,Z_Param_ParticipantTag);
	P_FINISH;
	P_NATIVE_BEGIN;
	UConversationContextHelpers::MakeConversationParticipant(Z_Param_Out_Context,Z_Param_ParticipantActor,Z_Param_ParticipantTag);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UConversationContextHelpers::execAdvanceConversation)
{
	P_GET_STRUCT_REF(FConversationContext,Z_Param_Out_Context);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FConversationTaskResult*)Z_Param__Result=UConversationContextHelpers::AdvanceConversation(Z_Param_Out_Context);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UConversationContextHelpers::execAdvanceConversationWithChoice)
{
	P_GET_STRUCT_REF(FConversationContext,Z_Param_Out_Context);
	P_GET_STRUCT_REF(FAdvanceConversationRequest,Z_Param_Out_Choice);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FConversationTaskResult*)Z_Param__Result=UConversationContextHelpers::AdvanceConversationWithChoice(Z_Param_Out_Context,Z_Param_Out_Choice);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UConversationContextHelpers::execPauseConversationAndSendClientChoices)
{
	P_GET_STRUCT_REF(FConversationContext,Z_Param_Out_Context);
	P_GET_STRUCT_REF(FClientConversationMessage,Z_Param_Out_Message);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FConversationTaskResult*)Z_Param__Result=UConversationContextHelpers::PauseConversationAndSendClientChoices(Z_Param_Out_Context,Z_Param_Out_Message);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UConversationContextHelpers::execReturnToLastClientChoice)
{
	P_GET_STRUCT_REF(FConversationContext,Z_Param_Out_Context);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FConversationTaskResult*)Z_Param__Result=UConversationContextHelpers::ReturnToLastClientChoice(Z_Param_Out_Context);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UConversationContextHelpers::execReturnToCurrentClientChoice)
{
	P_GET_STRUCT_REF(FConversationContext,Z_Param_Out_Context);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FConversationTaskResult*)Z_Param__Result=UConversationContextHelpers::ReturnToCurrentClientChoice(Z_Param_Out_Context);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UConversationContextHelpers::execReturnToConversationStart)
{
	P_GET_STRUCT_REF(FConversationContext,Z_Param_Out_Context);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(FConversationTaskResult*)Z_Param__Result=UConversationContextHelpers::ReturnToConversationStart(Z_Param_Out_Context);
	P_NATIVE_END;
}

void UConversationContextHelpers::Init() {
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationContextHelpers.MakeConversationParticipant"), execMakeConversationParticipant);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationContextHelpers.AdvanceConversation"), execAdvanceConversation);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationContextHelpers.AdvanceConversationWithChoice"), execAdvanceConversationWithChoice);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationContextHelpers.PauseConversationAndSendClientChoices"), execPauseConversationAndSendClientChoices);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationContextHelpers.ReturnToLastClientChoice"), execReturnToLastClientChoice);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationContextHelpers.ReturnToCurrentClientChoice"), execReturnToCurrentClientChoice);
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationContextHelpers.ReturnToConversationStart"), execReturnToConversationStart);
}

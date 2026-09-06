#include "pch.h"

void UFortNonPlayerConversationParticipantComponent::StartConversation(const FGameplayTag& InConversationEntryTag, AActor* Instigator, AActor* Target) const {
	UConversationLibrary::StartConversation(InConversationEntryTag, Instigator, InteractorParticipantTag, Target, SelfParticipantTag);
}

DEFINE_FUNCTION(UFortNonPlayerConversationParticipantComponent::execStartConversation)
{
	P_GET_STRUCT(FGameplayTag,Z_Param_InConversationEntryTag);
	P_GET_OBJECT(AActor,Z_Param_Instigator);
	P_GET_OBJECT(AActor,Z_Param_Target);
	P_FINISH;
	P_NATIVE_BEGIN;
	P_THIS_CAST(UFortNonPlayerConversationParticipantComponent)->StartConversation(Z_Param_InConversationEntryTag,Z_Param_Instigator,Z_Param_Target);
	P_NATIVE_END;
}

void UFortNonPlayerConversationParticipantComponent::Init() {
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function FortniteGame.FortNonPlayerConversationParticipantComponent.StartConversation"), execStartConversation);
}

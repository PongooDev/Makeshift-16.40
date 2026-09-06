#include "pch.h"

void UFortPlayerConversationComponent::RequestServerAbortConversation_Implementation() {
	ServerAbortAllConversations();
}

void UFortPlayerConversationComponent::RequestServerAbortConversationHook(UFortPlayerConversationComponent* This) {
	This->RequestServerAbortConversation_Implementation();
}

void UFortPlayerConversationComponent::Init() {
	Memory::SwapVTableEntryInAllSubClasses<UFortPlayerConversationComponent>(140, RequestServerAbortConversationHook);
}

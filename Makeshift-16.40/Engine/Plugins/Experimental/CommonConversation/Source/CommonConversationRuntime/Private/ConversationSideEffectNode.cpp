// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

void UConversationSideEffectNode::CauseSideEffect(const FConversationContext& Context) const
{
	if (Context.IsServerContext())
	{
		ServerCauseSideEffect(Context);
	}

	if (Context.IsClientContext())
	{
		ClientCauseSideEffect(Context);
	}
}

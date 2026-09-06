// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/ConversationTypes.h"
#include "Engine/Plugins/Experimental/CommonConversation/Source/CommonConversationRuntime/Public/CommonConversationRuntimeLogging.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"

//////////////////////////////////////////////////////////////////////////

UClass* UConversationSettings::GetConversationInstanceClass() const
{
	if (UClass* LoadedClass = ConversationInstanceClass.Get())
	{
		return LoadedClass;
	}

	const std::string AssetPath = ConversationInstanceClass.ObjectID.AssetPathName.ToString();
	const std::string ScriptPrefix = "/Script/";
	if (AssetPath.rfind(ScriptPrefix, 0) == 0)
	{
		return UObject::FindObject<UClass>("Class " + AssetPath.substr(ScriptPrefix.size()));
	}

	return nullptr;
}

UConversationInstance* UConversationLibrary::StartConversation(const FGameplayTag& ConversationEntryTag, AActor* Instigator, const FGameplayTag& InstigatorTag, AActor* Target, const FGameplayTag& TargetTag)
{
	if (Instigator == nullptr || Target == nullptr)
	{
		return nullptr;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(Instigator, EGetWorldErrorMode::LogAndReturnNull))
	{
		UClass* InstanceClass = UConversationSettings::GetDefaultObj()->GetConversationInstanceClass();
		if (InstanceClass == nullptr)
		{
			InstanceClass = UConversationInstance::StaticClass();
		}

		UConversationInstance* ConversationInstance = NewObject<UConversationInstance>(World, InstanceClass);
		if (ensure(ConversationInstance))
		{
			FConversationContext Context = FConversationContext::CreateServerContext(ConversationInstance, nullptr);

			UConversationContextHelpers::MakeConversationParticipant(Context, Target, TargetTag);
			UConversationContextHelpers::MakeConversationParticipant(Context, Instigator, InstigatorTag);

			ConversationInstance->ServerStartConversation(ConversationEntryTag);
		}

		return ConversationInstance;
	}

	return nullptr;
}

DEFINE_FUNCTION(UConversationLibrary::execStartConversation)
{
	P_GET_STRUCT(FGameplayTag,Z_Param_ConversationEntryTag);
	P_GET_OBJECT(AActor,Z_Param_Instigator);
	P_GET_STRUCT(FGameplayTag,Z_Param_InstigatorTag);
	P_GET_OBJECT(AActor,Z_Param_Target);
	P_GET_STRUCT(FGameplayTag,Z_Param_TargetTag);
	P_FINISH;
	P_NATIVE_BEGIN;
	*(UConversationInstance**)Z_Param__Result=UConversationLibrary::StartConversation(Z_Param_ConversationEntryTag,Z_Param_Instigator,Z_Param_InstigatorTag,Z_Param_Target,Z_Param_TargetTag);
	P_NATIVE_END;
}

void UConversationLibrary::Init() {
	Memory::HookUFunction(UObject::FindObject<UFunction>("Function CommonConversationRuntime.ConversationLibrary.StartConversation"), execStartConversation);
}

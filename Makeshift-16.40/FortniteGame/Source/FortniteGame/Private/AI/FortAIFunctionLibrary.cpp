#include "pch.h"

void UFortAIFunctionLibrary::SetHearingRange(AActor* AIAgent, float Range) {
	if (!AIAgent) {
		UE_LOG(LogFortAI, Warning, TEXT("Trying to set hearing range on None agent"));
		return;
	}

	AAIController* AIController = UAIBlueprintHelperLibrary::GetAIController(AIAgent);
	if (!AIController) {
		UE_LOG(LogFortAI, Warning, TEXT("Trying to set hearing range for %hs while it doesn't represent an AI agent"), AIAgent->GetName().c_str());
		return;
	}

	UFortAIPerceptionComponent* PerceptionComponent = AIController->PerceptionComponent ? AIController->PerceptionComponent->Cast<UFortAIPerceptionComponent>() : nullptr;
	if (!PerceptionComponent) {
		UE_LOG(LogFortAI, Warning, TEXT("Failed to find %hs's perception component. Bailing out"), AIAgent->GetName().c_str());
		return;
	}

	UAISenseConfig* SenseConfig = PerceptionComponent->GetSenseConfigForPerceptionType(ECorePerceptionTypes::Hearing);
	UAISenseConfig_Hearing* HearingConfig = SenseConfig ? SenseConfig->Cast<UAISenseConfig_Hearing>() : nullptr;
	if (!HearingConfig) {
		UE_LOG(LogFortAI, Warning, TEXT("Failed to find %hs's hearing sense config. Bailing out"), AIAgent->GetName().c_str());
		return;
	}

	HearingConfig->HearingRange = FMath::Max(Range, 0.f);
	PerceptionComponent->ConfigureSense(*HearingConfig);
}

DEFINE_FUNCTION(UFortAIFunctionLibrary::execSetHearingRange)
{
	P_GET_OBJECT(AActor,Z_Param_AIAgent);
	P_GET_PROPERTY(FFloatProperty,Z_Param_Range);
	P_FINISH;
	P_NATIVE_BEGIN;
	UFortAIFunctionLibrary::SetHearingRange(Z_Param_AIAgent,Z_Param_Range);
	P_NATIVE_END;
}

void UFortAIFunctionLibrary::Init() {
	Memory::HookDetour(ImageBase + 0x4F5DF94, execSetHearingRange, nullptr);
}

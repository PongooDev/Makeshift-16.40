// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_OFFSET(LogPhysics, Warning, All)
#define LogPhysics UE_LOG_CATEGORY_AT(LogPhysics, 0x94601F0)
DECLARE_LOG_CATEGORY_OFFSET(LogBlueprint, Warning, All)
#define LogBlueprint UE_LOG_CATEGORY_AT(LogBlueprint, 0x94584D8)
DECLARE_LOG_CATEGORY_OFFSET(LogBlueprintUserMessages, Log, All)
#define LogBlueprintUserMessages UE_LOG_CATEGORY_AT(LogBlueprintUserMessages, 0x94578D0)
DECLARE_LOG_CATEGORY_OFFSET(LogAnimation, Warning, All)
#define LogAnimation UE_LOG_CATEGORY_AT(LogAnimation, 0x9457A38)
DECLARE_LOG_CATEGORY_OFFSET(LogLevel, Log, All)
#define LogLevel UE_LOG_CATEGORY_AT(LogLevel, 0x945C630)
DECLARE_LOG_CATEGORY_OFFSET(LogNet, Log, All)
#define LogNet UE_LOG_CATEGORY_AT(LogNet, 0x9459308)
DECLARE_LOG_CATEGORY_OFFSET(LogRep, Log, All)
#define LogRep UE_LOG_CATEGORY_AT(LogRep, 0x9459318)
DECLARE_LOG_CATEGORY_OFFSET(LogNetTraffic, Warning, All)
#define LogNetTraffic UE_LOG_CATEGORY_AT(LogNetTraffic, 0x9459338)
DECLARE_LOG_CATEGORY_OFFSET(LogNetDormancy, Warning, All)
#define LogNetDormancy UE_LOG_CATEGORY_AT(LogNetDormancy, 0x9459358)
DECLARE_LOG_CATEGORY_OFFSET(LogNetFastTArray, Warning, All)
#define LogNetFastTArray UE_LOG_CATEGORY_AT(LogNetFastTArray, 0x9459368)
DECLARE_LOG_CATEGORY_OFFSET(LogSecurity, Warning, All)
#define LogSecurity UE_LOG_CATEGORY_AT(LogSecurity, 0x9459378)

DECLARE_LOG_CATEGORY_OFFSET(LogEngine, Log, All)
#define LogEngine UE_LOG_CATEGORY_AT(LogEngine, 0x9464520)
DECLARE_LOG_CATEGORY_OFFSET(LogWorld, Log, All)
#define LogWorld UE_LOG_CATEGORY_AT(LogWorld, 0x95ECB50)
DECLARE_LOG_CATEGORY_OFFSET(LogSpawn, Warning, All)
#define LogSpawn UE_LOG_CATEGORY_AT(LogSpawn, 0x9465160)
DECLARE_LOG_CATEGORY_OFFSET(LogActor, Log, Warning)
#define LogActor UE_LOG_CATEGORY_AT(LogActor, 0x9457830)
DECLARE_LOG_CATEGORY_OFFSET(LogGameMode, Log, All)
#define LogGameMode UE_LOG_CATEGORY_AT(LogGameMode, 0x945A930)
DECLARE_LOG_CATEGORY_OFFSET(LogGameState, Log, All)
#define LogGameState UE_LOG_CATEGORY_AT(LogGameState, 0x945A960)
DECLARE_LOG_CATEGORY_OFFSET(LogGameSession, Log, All)
#define LogGameSession UE_LOG_CATEGORY_AT(LogGameSession, 0x945A950)
DECLARE_LOG_CATEGORY_OFFSET(LogPlayerController, Log, All)
#define LogPlayerController UE_LOG_CATEGORY_AT(LogPlayerController, 0x9460BA0)

// Copyright Epic Games, Inc. All Rights Reserved.

#include "pch.h"
#include "Engine/Source/Runtime/Core/Public/HAL/IConsoleManager.h"

IConsoleManager*& IConsoleManager::Singleton = *reinterpret_cast<IConsoleManager**>(InSDKUtils::GetImageBase() + 0x9365968);

void IConsoleManager::SetupSingleton()
{
}

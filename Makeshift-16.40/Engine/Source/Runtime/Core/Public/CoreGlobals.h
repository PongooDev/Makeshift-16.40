// Copyright Epic Games, Inc. All Rights Reserved.

// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreTypes.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_OFFSET(LogHAL, Log, All)
#define LogHAL UE_LOG_CATEGORY_AT(LogHAL, 0x93DBBB8)
DECLARE_LOG_CATEGORY_OFFSET(LogNetPackageMap, Warning, All)
#define LogNetPackageMap UE_LOG_CATEGORY_AT(LogNetPackageMap, 0x93DBBE8)
DECLARE_LOG_CATEGORY_OFFSET(LogCore, Log, All)
#define LogCore UE_LOG_CATEGORY_AT(LogCore, 0x93DBD50)
DECLARE_LOG_CATEGORY_OFFSET(LogOutputDevice, Log, All)
#define LogOutputDevice UE_LOG_CATEGORY_AT(LogOutputDevice, 0x93DC6C0)
DECLARE_LOG_CATEGORY_OFFSET(LogStreaming, Display, All)
#define LogStreaming UE_LOG_CATEGORY_AT(LogStreaming, 0x93DBCC0)
DECLARE_LOG_CATEGORY_OFFSET(LogInit, Log, All)
#define LogInit UE_LOG_CATEGORY_AT(LogInit, 0x93DBCD0)
DECLARE_LOG_CATEGORY_OFFSET(LogExit, Log, All)
#define LogExit UE_LOG_CATEGORY_AT(LogExit, 0x93DBCE0)
DECLARE_LOG_CATEGORY_OFFSET(LogExec, Warning, All)
#define LogExec UE_LOG_CATEGORY_AT(LogExec, 0x93DBCF0)
DECLARE_LOG_CATEGORY_OFFSET(LogScript, Warning, All)
#define LogScript UE_LOG_CATEGORY_AT(LogScript, 0x93DBD00)
DECLARE_LOG_CATEGORY_OFFSET(LogLoad, Log, All)
#define LogLoad UE_LOG_CATEGORY_AT(LogLoad, 0x93DBD40)
DECLARE_LOG_CATEGORY_OFFSET(LogTemp, Log, All)
#define LogTemp UE_LOG_CATEGORY_AT(LogTemp, 0x93DBC28)

DECLARE_LOG_CATEGORY_OFFSET(LogConfig, Log, All)
#define LogConfig UE_LOG_CATEGORY_AT(LogConfig, 0x93DBA18)
DECLARE_LOG_CATEGORY_OFFSET(LogNetVersion, Log, All)
#define LogNetVersion UE_LOG_CATEGORY_AT(LogNetVersion, 0x93DC6B0)

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

class FOutputDeviceRedirector;
class FOutputDeviceError;
class FFeedbackContext;

CORE_API FOutputDeviceRedirector* GetGlobalLogSingleton();

#define GLog GetGlobalLogSingleton()

inline FOutputDeviceError*&	GError = *reinterpret_cast<FOutputDeviceError**>(InSDKUtils::GetImageBase() + 0x938EE70);
inline FFeedbackContext*&	GWarn = *reinterpret_cast<FFeedbackContext**>(InSDKUtils::GetImageBase() + 0x938EEC8);

inline TCHAR (&GErrorHist)[16384] = *reinterpret_cast<TCHAR(*)[16384]>(InSDKUtils::GetImageBase() + 0x9386E40);

inline bool& GIsClient = *reinterpret_cast<bool*>(InSDKUtils::GetImageBase() + 0x938EE5B);
inline bool& GIsServer = *reinterpret_cast<bool*>(InSDKUtils::GetImageBase() + 0x938EE5A);
inline bool& GIsCriticalError = *reinterpret_cast<bool*>(InSDKUtils::GetImageBase() + 0x938EE80);

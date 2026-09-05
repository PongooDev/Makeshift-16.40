// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <format>
#include <print>
#include <array>
#include <TlHelp32.h>
#include <dbghelp.h>
#include <sstream>
#include <winternl.h>
#include <cstdlib>
#include <cmath>
#include <intrin.h>
#include <windows.h>
#include <psapi.h>
#include <unordered_map>
#include <numeric>
#include <cwctype>
#include <cstdarg>
#include <unordered_set>
#include <algorithm>
#include <cwctype>

#include "minhook/MinHook.h"

#pragma comment(lib, "minhook/minhook.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

inline uint64_t ImageBase = reinterpret_cast<uint64_t>(GetModuleHandleW(nullptr));

#include "SDK/PropertyFixup.hpp"
#include "SDK/UnrealContainers.hpp"

using namespace UC;

#include "Engine/Source/Runtime/Engine/Classes/Engine/EngineBaseTypes.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/NetConnection.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Channel.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/ActorChannel.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/CoreNetTypes.h"
#include "Engine/Source/Runtime/Core/Public/Misc/NetworkGuid.h"
#include "Engine/Source/Runtime/Core/Public/Templates/SharedPointer.h"
#include "Engine/Source/Runtime/Core/Public/Templates/UniquePtr.h"
#include "Engine/Source/Runtime/Engine/Public/Net/NetAnalyticsTypes.h"

struct FActorPriority;
struct FActorDestructionInfo;
struct FNetworkObjectInfo;
class FNetworkObjectList;
class FNetGUIDCache;
class FObjectReplicator;

#include "SDK/SDK.hpp"

using namespace SDK;

namespace Configuration {
	inline bool bUseGameSessions = false;

	inline std::wstring Playlist = L"Playlist_DefaultSolo";
	inline std::string MapToLoad = "Apollo_Terrain";
}


#include "Engine/Source/Runtime/Core/Public/UObject/UnrealNames.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogMacros.h"
#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"
#include "Engine/Source/Runtime/Core/Public/Logging/LogScopedVerbosityOverride.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDeviceRedirector.h"
#include "Engine/Source/Runtime/Core/Public/Misc/OutputDeviceError.h"
#include "Engine/Source/Runtime/Core/Public/Misc/FeedbackContext.h"
#include "Engine/Source/Runtime/Core/Public/Misc/CString.h"
#include "Engine/Source/Runtime/Core/Public/Templates/UnrealTemplate.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/EngineLogs.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/Engine.h"
#include "Engine/Source/Runtime/Engine/Classes/Engine/World.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectBaseUtility.h"

DECLARE_LOG_CATEGORY_OFFSET(LogOnline, Log, All)
#define LogOnline UE_LOG_CATEGORY_AT(LogOnline, 0x93A6618)
DECLARE_LOG_CATEGORY_OFFSET(LogOnlineGame, Log, All)
#define LogOnlineGame UE_LOG_CATEGORY_AT(LogOnlineGame, 0x93A6628)
DECLARE_LOG_CATEGORY_OFFSET(LogBeacon, Log, All)
#define LogBeacon UE_LOG_CATEGORY_AT(LogBeacon, 0x93A69A0)
DECLARE_LOG_CATEGORY_OFFSET(LogAbilitySystem, Display, All)
#define LogAbilitySystem UE_LOG_CATEGORY_AT(LogAbilitySystem, 0x93A5740)
DECLARE_LOG_CATEGORY_OFFSET(LogAbilitySystemComponent, Log, All)
#define LogAbilitySystemComponent UE_LOG_CATEGORY_AT(LogAbilitySystemComponent, 0x93A56E0)
DECLARE_LOG_CATEGORY_OFFSET(LogReplicationGraph, Log, All)
#define LogReplicationGraph UE_LOG_CATEGORY_AT(LogReplicationGraph, 0x93AD0E0)
DECLARE_LOG_CATEGORY_OFFSET(LogFort, Log, All)
#define LogFort UE_LOG_CATEGORY_AT(LogFort, 0x93CAB28)
DECLARE_LOG_CATEGORY_OFFSET(LogFortInventory, Warning, All)
#define LogFortInventory UE_LOG_CATEGORY_AT(LogFortInventory, 0x93CAB38)
DECLARE_LOG_CATEGORY_OFFSET(LogFortBuilding, Log, All)
#define LogFortBuilding UE_LOG_CATEGORY_AT(LogFortBuilding, 0x93CAB68)
DECLARE_LOG_CATEGORY_OFFSET(LogFortReplicationGraph, Display, All)
#define LogFortReplicationGraph UE_LOG_CATEGORY_AT(LogFortReplicationGraph, 0x93CC700)

DECLARE_LOG_CATEGORY_EXTERN(LogMakeshift, Log, All);

inline void InitConsole() {
	AllocConsole();
	FILE* fptr;
	freopen_s(&fptr, "CONOUT$", "w+", stdout);
}

inline void VARARGS LogImpl(ELogVerbosity::Type Verbosity, const TCHAR* Fmt, ...)
{
	if (LogMakeshift.IsSuppressed(Verbosity))
	{
		return;
	}

	TCHAR Buffer[8192];
	GET_VARARGS(Buffer, UE_ARRAY_COUNT(Buffer), UE_ARRAY_COUNT(Buffer) - 1, Fmt, Fmt);

	if ((Verbosity & ELogVerbosity::VerbosityMask) == ELogVerbosity::Fatal)
	{
		FMsg::Logf_Internal(__FILE__, __LINE__, LogMakeshift.GetCategoryName(), ELogVerbosity::Fatal, TEXT("%s"), Buffer);
		_DebugBreakAndPromptForRemote();
		FDebug::ProcessFatalError();
		return;
	}

	FMsg::Logf(__FILE__, __LINE__, LogMakeshift.GetCategoryName(), Verbosity, TEXT("%s"), Buffer);
}

template <typename... Types>
inline void Log(ELogVerbosity::Type Verbosity, const TCHAR* Fmt, Types... Args)
{
	static_assert(TAnd<TIsValidVariadicFunctionArg<Types>...>::Value, "Invalid argument(s) passed to Log");

	LogImpl(Verbosity, Fmt, Args...);
}

template <typename... Types>
inline void Log(const TCHAR* Fmt, Types... Args)
{
	Log(ELogVerbosity::Display, Fmt, Args...);
}

inline std::wstring ToWide(const char* Str)
{
	if (!Str || !*Str)
		return {};

	const int Length = MultiByteToWideChar(CP_UTF8, 0, Str, -1, nullptr, 0);
	if (Length <= 1)
		return {};

	std::wstring Out(static_cast<size_t>(Length) - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, Str, -1, Out.data(), Length);
	return Out;
}

inline std::wstring NarrowFormatToWide(const char* Fmt)
{
	std::wstring Out = ToWide(Fmt);
	for (size_t i = 0; i < Out.size(); ++i)
	{
		if (Out[i] != L'%')
			continue;

		size_t j = i + 1;
		if (j < Out.size() && Out[j] == L'%')
		{
			i = j;
			continue;
		}

		while (j < Out.size() && wcschr(L"-+ #0123456789.*", Out[j]))
			++j;

		if (j < Out.size() && (Out[j] == L's' || Out[j] == L'c') && !wcschr(L"hlLqjzt", Out[j - 1]))
		{
			Out.insert(j, 1, L'h');
		}
		i = j;
	}
	return Out;
}

template <typename... Types>
inline void Log(ELogVerbosity::Type Verbosity, const char* Fmt, Types... Args)
{
	Log(Verbosity, NarrowFormatToWide(Fmt).c_str(), Args...);
}

template <typename... Types>
inline void Log(const char* Fmt, Types... Args)
{
	Log(ELogVerbosity::Display, NarrowFormatToWide(Fmt).c_str(), Args...);
}

inline void Log(ELogVerbosity::Type Verbosity, const std::string& Message)
{
	Log(Verbosity, TEXT("%hs"), Message.c_str());
}

inline void Log(const std::string& Message)
{
	Log(ELogVerbosity::Display, TEXT("%hs"), Message.c_str());
}

class Memory
{
public:
	template <typename _Is = uint8_t>
	static void Patch(uintptr_t ptr, _Is byte)
	{
		DWORD og;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), PAGE_EXECUTE_READWRITE, &og);
		*(_Is*)ptr = byte;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), og, &og);

		FlushInstructionCache(GetCurrentProcess(), LPCVOID(ptr), sizeof(_Is));
	}

	static void Nop(uintptr_t ptr, size_t size)
	{
		DWORD og;
		VirtualProtect(LPVOID(ptr), size, PAGE_EXECUTE_READWRITE, &og);
		memset(LPVOID(ptr), 0x90, size);
		VirtualProtect(LPVOID(ptr), size, og, &og);

		FlushInstructionCache(GetCurrentProcess(), LPCVOID(ptr), size);
	}

	static void NopFunctionCall(uintptr_t callSite)
	{
		if (*reinterpret_cast<uint8_t*>(callSite) != 0xE8)
			return;

		Nop(callSite, 5);
	}

	static void SwapVTableEntry(void** VTable, int Idx, void* Detour, void** OG = 0)
	{
		DWORD oldProtection;

		if (OG)
		{
			*OG = VTable[Idx];
		}

		VirtualProtect(&VTable[Idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);

		VTable[Idx] = Detour;

		VirtualProtect(&VTable[Idx], sizeof(void*), oldProtection, NULL);
	}

	static void ChangeVFTCallClass(UClass* OriginalClass, uint32_t Idx, UClass* NewClass)
	{
		SwapVTableEntry(OriginalClass->DefaultObject->VTable, Idx, OriginalClass->DefaultObject->VTable[Idx]);
	}

	static void HookDetour(uintptr_t Ptr, void* Detour, void* Original = nullptr)
	{
		static bool check = false;
		if (!check)
		{
			MH_Initialize();
			check = !check;
		}
		MH_CreateHook(LPVOID(Ptr), Detour, (void**)Original);
		MH_EnableHook(LPVOID(Ptr));
	}

	static inline void ModifyInstructionLEA(uintptr_t instrAddr, uintptr_t targetAddr, int offset)
	{
		int64_t delta = static_cast<int64_t>(targetAddr) -
			static_cast<int64_t>(instrAddr + offset + 4);

		auto patchLocation = reinterpret_cast<int32_t*>(instrAddr + offset);

		DWORD oldProtect;
		VirtualProtect(patchLocation, sizeof(int32_t), PAGE_EXECUTE_READWRITE, &oldProtect);

		*patchLocation = static_cast<int32_t>(delta);

		DWORD temp;
		VirtualProtect(patchLocation, sizeof(int32_t), oldProtect, &temp);
	}

	static void HookUFunction(UFunction* Function, void* Detour, void** Original = nullptr)
	{
		if (Original) *Original = (void*)Function->ExecFunction;
		Function->ExecFunction = reinterpret_cast<UFunction::FNativeFuncPtr>(Detour);
	}

	template<typename T>
	static void SwapVTableEntryInAllSubClasses(uint32_t Idx, void* Detour)
	{
		auto BigC = T::StaticClass();
		for (int i = 0; i < UObject::GObjects->Num(); ++i)
		{
			auto Obj = UObject::GObjects->GetByIndex(i);
			if (Obj)
			{
				if (Obj->IsDefaultObject())
				{
					if (Obj->IsA(BigC))
					{
						HookVTable(Obj, Idx, Detour, 0);
					}
				}
			}
		}
	}

	static bool PatchBytes(void* address, const void* bytes, size_t size)
	{
		if (!address || !bytes || size == 0)
			return false;

		DWORD oldProtect{};
		if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
			return false;

		std::memcpy(address, bytes, size);
		FlushInstructionCache(GetCurrentProcess(), address, size);

		DWORD temp{};
		VirtualProtect(address, size, oldProtect, &temp);
		return true;
	}

	static bool PatchByte(void* address, uint8_t value)
	{
		return PatchBytes(address, &value, sizeof(value));
	}

	static bool PatchByte(uintptr_t address, uint8_t value)
	{
		return PatchByte(reinterpret_cast<void*>(address), value);
	}

	static bool PatchByteChecked(uintptr_t address, uint8_t expected, uint8_t value)
	{
		auto ptr = reinterpret_cast<uint8_t*>(address);
		if (!ptr || *ptr != expected)
			return false;

		return PatchByte(address, value);
	}

	static bool NopBytes(void* address, size_t count)
	{
		if (!address || count == 0)
			return false;

		std::vector<uint8_t> nops(count, 0x90);
		return PatchBytes(address, nops.data(), nops.size());
	}

	static bool NopBytes(uintptr_t address, size_t count)
	{
		return NopBytes(reinterpret_cast<void*>(address), count);
	}

	static bool PatchCall(uintptr_t callSite, void* newTarget)
	{
		if (!callSite || !newTarget)
			return false;

		int32_t relOffset = (int32_t)((uintptr_t)newTarget - (callSite + 5));

		uint8_t patch[5];
		patch[0] = 0xE8;
		memcpy(&patch[1], &relOffset, sizeof(relOffset));

		return PatchBytes(reinterpret_cast<void*>(callSite), patch, sizeof(patch));
	}

	static bool PatchCallFar(uintptr_t callSite, void* newTarget)
	{
		uintptr_t base = callSite & ~0xFFFFull;
		void* trampoline = nullptr;

		for (uintptr_t addr = base; addr > base - 0x80000000ull; addr -= 0x10000)
		{
			trampoline = VirtualAlloc((void*)addr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (trampoline) break;
		}

		if (!trampoline)
			return false;

		uint8_t jmp[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		memcpy(&jmp[6], &newTarget, sizeof(newTarget));
		memcpy(trampoline, jmp, sizeof(jmp));

		return PatchCall(callSite, trampoline);
	}

	static void HookVTable(void* Base, int Idx, void* Detour, void** OG = nullptr)
	{
		if (!Base || !Detour)
		{
			return;
		}
		DWORD oldProtection;

		void** VTable = *(void***)Base;

		if (OG)
		{
			*OG = VTable[Idx];
		}

		VirtualProtect(&VTable[Idx], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtection);

		VTable[Idx] = Detour;

		VirtualProtect(&VTable[Idx], sizeof(void*), oldProtection, NULL);
	}

	static void CreateVTableOriginal(void* Base, int Idx, void** Original) {
		if (!Base)
		{
			return;
		}
		DWORD oldProtection;

		void** VTable = *(void***)Base;

		if (Original)
		{
			*Original = VTable[Idx];
		}
	}
};

static void RetNull() {}

static int True() {
	return 1;
}

static int False() {
	return 0;
}

#endif //PCH_H

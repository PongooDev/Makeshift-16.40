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
#include <minmax.h>
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

#include "minhook/MinHook.h"

#pragma comment(lib, "minhook/minhook.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

#include "SDK/SDK.hpp"

using namespace SDK;

namespace Configuration {
	inline bool bUseGameSessions = false;

	inline std::wstring Playlist = L"Playlist_DefaultSolo";
	inline std::string MapToLoad = "Apollo_Terrain";
}

inline uint64_t ImageBase = InSDKUtils::GetImageBase();

inline void InitConsole() {
	AllocConsole();
	FILE* fptr;
	freopen_s(&fptr, "CONOUT$", "w+", stdout);
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

// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"

DEFINE_LOG_CATEGORY(LogMakeshift);

DWORD WINAPI Main(LPVOID) {
    InitConsole();

    UE_LOG(LogMakeshift, Display, TEXT("Makeshift 16.40 loaded, ImageBase: 0x%llX"), ImageBase);
    Log(std::format("Playlist: {}, Map: {}", std::string(Configuration::Playlist.begin(), Configuration::Playlist.end()), Configuration::MapToLoad));

    if (GIsClient) {
        GIsClient = 0;
    }
    if (!GIsServer) {
        GIsServer = 1;
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Main, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

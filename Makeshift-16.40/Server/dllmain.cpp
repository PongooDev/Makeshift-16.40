// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Engine/Source/Runtime/Core/Public/CoreGlobals.h"

DEFINE_LOG_CATEGORY(LogMakeshift);

DWORD WINAPI Main(LPVOID) {
    InitConsole();

    UE_LOG(LogMakeshift, Display, TEXT("Makeshift 16.40 loaded, ImageBase: 0x%llX"), ImageBase);
    Log(std::format("Playlist: {}, Map: {}", std::string(Configuration::Playlist.begin(), Configuration::Playlist.end()), Configuration::MapToLoad));

    while (true)
    {
        if (GWorld)
        {
            if (GWorld->GetName() == "Frontend")
            {
                if (GWorld->AuthorityGameMode)
                {
                    if (auto GM = GWorld->AuthorityGameMode->Cast<AGameMode>())
                    {
                        if (GM->MatchState == FName(L"InProgress"))
                        {
                            break;
                        }
                    }
                }
            }
        }

        Sleep(1000);
    }

    if (GIsClient) {
        GIsClient = 0;
    }
    if (!GIsServer) {
        GIsServer = 1;
    }

    UWorld::Init();
    UEngine::Init();
    AFortGameSession::Init();
    UNetDriver::Init();

    UWorld* World = GWorld;
    if (World) {
        UFortGameInstance* FortGameInstance = World->OwningGameInstance->Cast<UFortGameInstance>();

        const bool bPlaylistIsId = !Configuration::Playlist.empty() && std::all_of(Configuration::Playlist.begin(), Configuration::Playlist.end(), ::iswdigit);

        std::wstring TravelURL = ToWide(Configuration::MapToLoad.c_str()) + L"?listen?RequiredPlayers=1";
        TravelURL += (bPlaylistIsId ? L"?playlistId=" : L"?Playlist=") + Configuration::Playlist;

        if (FortGameInstance && FortGameInstance->LocalPlayers.Num()) {
            FortGameInstance->LocalPlayers.Remove(0);
        }

        UE_LOG(LogMakeshift, Display, TEXT("ServerTravel to %s"), TravelURL.c_str());
        World->ServerTravel(FString(TravelURL.c_str()));
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

#include "pch.h"
#include <Windows.h>
#include "Core/Game.h"


DWORD WINAPI InitThread(LPVOID)
{
    Game::Initialize();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
    }
    return TRUE;
}

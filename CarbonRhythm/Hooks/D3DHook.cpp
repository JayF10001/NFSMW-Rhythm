#include "Hooks/D3DHook.h"
#include "Rhythm/Input.h"
#include "Rhythm/RhythmSystem.h"
#include "Core/Game.h"
#include "Render/Overlay.h"
#include <MinHook.h>
#include <Audio/AudioSystem.h>
#include <Rhythm/BeatmapLoader.h>
#include "Car/DanceController.h"
#include <Core/GameTime.h>
#include "Rhythm/BeatmapRecorder.h"
#include <Core/Config.h>
#include "UI/MenuSystem.h"
#include <Core/GameState.h>




typedef HRESULT(__stdcall* EndScene_t)(IDirect3DDevice9*);
EndScene_t oEndScene = nullptr;

// #define CARBON_DEVICE_PTR 0x00AB0ABC // nfsc
#define CARBON_DEVICE_PTR 0x00982BDC // nfsmw

HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice)
{
    static bool gInitialized = false;
    static bool gPrevMenuToggle = false;
    static bool gRecordMode = false;
    static bool gIsPlaying = false;

    const auto& keys = Config::GetKeys();
    const auto& rec = Config::GetRecorder();

    HRESULT result = oEndScene(pDevice);

    // ===== time delta =====
    static uint64_t lastCounter = 0;
    float dt = GameTime::DeltaSeconds(lastCounter);

    Input::Update();

    static bool gPrevToggle = false;

    bool toggle = (GetAsyncKeyState(keys.MenuToggle) & 0x8000) != 0;

    if (toggle && !gPrevToggle)
    {
        if (Game::GetState() == GameState::Inactive)
        {
            Game::SetState(GameState::Menu);
        }
        else if (Game::GetState() == GameState::Menu)
        {
            Game::SetState(GameState::Inactive);
        }
        else if (Game::GetState() == GameState::Playing)
        {
            Audio::Stop();
            Dance::SetEnabled(false);
            Game::SetState(GameState::Menu);
        }
    }

    gPrevToggle = toggle;

    switch (Game::GetState())
    {
    case GameState::Inactive:
        break; // do nothing

    case GameState::Menu:
        Menu::Update(dt);
        Menu::Render(pDevice);
        break;

    case GameState::Playing:
        Rhythm::Update();
        Dance::Update(dt);
        Overlay::Render(pDevice);
        break;

    case GameState::Recording:
        Recorder::Update();

        if (Recorder::GetState() == Recorder::RecorderState::Idle)
        {
            Recorder::RenderInfo(pDevice);
        }
        else
        {
            Dance::Update(dt);
            Overlay::Render(pDevice);
        }
        break;


    return result;
    }
}



typedef HRESULT(__stdcall* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset_t oReset = nullptr;

HRESULT __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
{
    Overlay::OnLostDevice();
    Menu::OnLostDevice();
    Recorder::OnLostDevice();


    HRESULT hr = oReset(device, pp);

    Overlay::OnResetDevice();
    Menu::OnResetDevice();
    Recorder::OnResetDevice();


    return hr;
}



void D3DHook::Initialize()
{
    IDirect3DDevice9* pDevice = nullptr;

    while (!pDevice)
    {
        Sleep(100);
        auto devicePtr = reinterpret_cast<IDirect3DDevice9**>(CARBON_DEVICE_PTR);
        if (devicePtr)
            pDevice = *devicePtr;
    }

    void** vtable = *reinterpret_cast<void***>(pDevice);

    char path[MAX_PATH];
    HMODULE hModule = nullptr;
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCSTR)&hkEndScene,
        &hModule
    );

    GetModuleFileNameA(hModule, path, MAX_PATH);

    Config::Load();

    const auto& keys = Config::GetKeys();
    for (int i = 0; i < 6; i++)
    {
        Input::SetKey(i, keys.Lane[i]);
    }



    MH_Initialize();
    MH_CreateHook(vtable[42], hkEndScene, reinterpret_cast<void**>(&oEndScene));
    MH_CreateHook(vtable[16], hkReset, reinterpret_cast<void**>(&oReset));
    MH_EnableHook(MH_ALL_HOOKS);
    
    
}

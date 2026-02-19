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



typedef HRESULT(__stdcall* EndScene_t)(IDirect3DDevice9*);
EndScene_t oEndScene = nullptr;

#define CARBON_DEVICE_PTR 0x00AB0ABC

HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice)
{
    static bool gSystemActive = false;
    static bool gPrevF2 = false;
    static bool gInitialized = false;

    static bool gRecordMode = false;
    static bool gPrevF3 = false;
    static bool gPrevF4 = false;

    HRESULT result = oEndScene(pDevice);

    bool f2 = (GetAsyncKeyState(VK_F2) & 0x8000) != 0;
    bool f3 = (GetAsyncKeyState(VK_F3) & 0x8000) != 0;
    bool f4 = (GetAsyncKeyState(VK_F4) & 0x8000) != 0;

    // ===== INIT AUDIO ONCE =====
    if (!gInitialized)
    {
        Audio::Initialize();
        gInitialized = true;
    }

    // ==============================
    // ===== PLAY MODE TOGGLE (F2)
    // ==============================
    if (f2 && !gPrevF2 && !gRecordMode)
    {
        gSystemActive = !gSystemActive;

        if (gSystemActive)
        {
            Rhythm::Initialize();
            Beatmap::Load("CarbonRhythmAssets/maps/sophie.json");
            Audio::Play();
            Dance::SetEnabled(true);
        }
        else
        {
            Audio::Stop();
            Rhythm::Initialize();
            Dance::SetEnabled(false);
        }
    }

    gPrevF2 = f2;

    // ==============================
    // ===== RECORD MODE START (F3)
    // ==============================
    if (f3 && !gPrevF3 && !gSystemActive)
    {
        gRecordMode = true;

        Rhythm::Initialize();
        Audio::Stop();
        Audio::Play();

        Recorder::Start(120.0, -0.05);
    }

    // ==============================
    // ===== RECORD MODE STOP (F4)
    // ==============================
    if (f4 && !gPrevF4 && gRecordMode)
    {
        gRecordMode = false;
        Recorder::Stop("CarbonRhythmAssets/maps/output.json");
        Audio::Stop();
    }

    gPrevF3 = f3;
    gPrevF4 = f4;

    // ==================================
    // ===== FRAME UPDATE SECTION
    // ==================================

    static uint64_t lastTick = 0;
    uint64_t tick = bGetTicker();

    if (lastTick == 0)
        lastTick = tick;

    uint64_t delta = tick - lastTick;
    lastTick = tick;

    float dt = (delta * TicksToMilliseconds) / 1000.0f;

    Input::Update();

    if (gRecordMode)
    {
        Recorder::Update();
    }
    else if (gSystemActive)
    {
        Rhythm::Update();
        Dance::Update(dt);
        Overlay::Render(pDevice);
    }

    return result;
}


typedef HRESULT(__stdcall* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset_t oReset = nullptr;

HRESULT __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp)
{
    Overlay::OnLostDevice();

    HRESULT hr = oReset(device, pp);

    Overlay::OnResetDevice();

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


    char* p = strrchr(path, '\\');
    if (p) *(p + 1) = 0;
    lstrcatA(path, "CarbonRhythm.ini");

    char buf[8];

    int slowSpin = GetPrivateProfileIntA("Controls", "SlowSpin", 85, path);
    Input::SetKey(0, slowSpin);

    int left = GetPrivateProfileIntA("Controls", "Left", 74, path);
    Input::SetKey(1, left);

    int down = GetPrivateProfileIntA("Controls", "Down", 75, path);
    Input::SetKey(2, down);

    int up = GetPrivateProfileIntA("Controls", "Up", 73, path);
    Input::SetKey(3, up);

    int right = GetPrivateProfileIntA("Controls", "Right", 76, path);
    Input::SetKey(4, right);

    int quickSpin = GetPrivateProfileIntA("Controls", "QuickSpin", 79, path);
    Input::SetKey(5, quickSpin);




    MH_Initialize();
    MH_CreateHook(vtable[42], hkEndScene, reinterpret_cast<void**>(&oEndScene));
    MH_CreateHook(vtable[16], hkReset, reinterpret_cast<void**>(&oReset));
    MH_EnableHook(MH_ALL_HOOKS);
    
    
}


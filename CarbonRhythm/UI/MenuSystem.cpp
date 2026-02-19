#include "MenuSystem.h"
#include <Windows.h>
#include <d3dx9.h>
#include <vector>
#include <string>
#include "../Core/Config.h"
#include <fstream>
#include <External/json.hpp>
#include <Rhythm/BeatmapLoader.h>
#include <Rhythm/RhythmSystem.h>
#include <Audio/AudioSystem.h>
#include <Car/DanceController.h>
#include <Core/GameState.h>
#include <Rhythm/BeatmapRecorder.h>

using json = nlohmann::json;
static LPD3DXSPRITE gSprite = nullptr;

namespace
{
    struct BeatmapEntry
    {
        std::string filePath;
        std::string title;
        std::string artist;
        std::string difficulty;
    };

    static std::vector<BeatmapEntry> gBeatmaps;
    static int gBeatmapSelection = 0;

    enum class MenuState
    {
        Main,
        BeatmapSelect
    };

    static MenuState gState = MenuState::Main;
    static int gSelection = 0;

    static LPD3DXFONT gFont = nullptr;

    static std::vector<std::string> gMainItems =
    {
        "Select Beatmap",
        "Beatmap Recorder",
        "Reload Config"
    };

    static bool gPrevUp = false;
    static bool gPrevDown = false;
    static bool gPrevLeft = false;
    static bool gPrevRight = false;

    static void ScanBeatmaps()
    {
        gBeatmaps.clear();
        gBeatmapSelection = 0;

        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(
            "CarbonRhythmAssets\\maps\\*.json",
            &findData
        );

        if (hFind == INVALID_HANDLE_VALUE)
            return;

        do
        {
            std::string fileName = findData.cFileName;
            std::string fullPath =
                "CarbonRhythmAssets\\maps\\" + fileName;

            std::ifstream file(fullPath);
            if (!file.is_open())
                continue;

            json j;
            file >> j;

            std::string title = j.value("title", fileName);
            std::string artist = j.value("artist", "Unknown");
            std::string diff = j.value("difficultyName", "Normal");

            gBeatmaps.push_back({ fullPath, title, artist, diff });

        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    }
}

namespace Menu
{
    void Initialize()
    {
        gState = MenuState::Main;
        gSelection = 0;
    }

    void Update(float dt)
    {
        const auto& keys = Config::GetKeys();

        bool up = (GetAsyncKeyState(keys.MenuUp) & 0x8000) != 0;
        bool down = (GetAsyncKeyState(keys.MenuDown) & 0x8000) != 0;
        bool left = (GetAsyncKeyState(keys.MenuLeft) & 0x8000) != 0;
        bool right = (GetAsyncKeyState(keys.MenuRight) & 0x8000) != 0;

        if (gState == MenuState::Main)
        {
            if (up && !gPrevUp)
            {
                gSelection--;
                if (gSelection < 0)
                    gSelection = (int)gMainItems.size() - 1;
            }

            if (down && !gPrevDown)
            {
                gSelection++;
                if (gSelection >= (int)gMainItems.size())
                    gSelection = 0;
            }

            if (right && !gPrevRight)
            {
                if (gSelection == 0)
                {
                    gState = MenuState::BeatmapSelect;
                    ScanBeatmaps();
                }
                else if (gSelection == 1)
                {
                    Recorder::EnterIdle();
                    Game::SetState(GameState::Recording);
                }
                else if (gSelection == 2)
                {
                    Config::Reload();
                }
            }
        }
        else if (gState == MenuState::BeatmapSelect)
        {
            if (up && !gPrevUp)
            {
                gBeatmapSelection--;
                if (gBeatmapSelection < 0)
                    gBeatmapSelection = (int)gBeatmaps.size() - 1;
            }

            if (down && !gPrevDown)
            {
                gBeatmapSelection++;
                if (gBeatmapSelection >= (int)gBeatmaps.size())
                    gBeatmapSelection = 0;
            }

            if (right && !gPrevRight)
            {
                if (!gBeatmaps.empty())
                {
                    const auto& selected = gBeatmaps[gBeatmapSelection];

                    Rhythm::Initialize();   // reset dulu
                    Beatmap::Load(selected.filePath);  // load notes + audio
                    Dance::SetEnabled(true);

                    Game::SetState(GameState::Playing);
                }
            }
            if (left && !gPrevLeft)
            {
                gState = MenuState::Main;
            }
        }


        gPrevUp = up;
        gPrevDown = down;
        gPrevLeft = left;
        gPrevRight = right;
    }

    void Render(IDirect3DDevice9* device)
    {
        

        // ===== INIT FONT =====
        if (!gFont)
        {
            D3DXCreateFont(
                device,
                22,
                0,
                FW_BOLD,
                1,
                FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE,
                L"Arial",
                &gFont
            );
        }

        // ===== INIT SPRITE =====
        if (!gSprite)
        {
            D3DXCreateSprite(device, &gSprite);
        }

        // ===== BEGIN DRAW =====
        gSprite->Begin(D3DXSPRITE_ALPHABLEND);

        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        gSprite->SetTransform(&identity);

        if (gState == MenuState::Main)
        {
            RECT titleRect = { 100, 80, 400, 120 };

            gFont->DrawTextA(
                gSprite,
                "CarbonRhythm v0.x",
                -1,
                &titleRect,
                DT_LEFT | DT_NOCLIP,
                D3DCOLOR_ARGB(255, 255, 255, 0)
            );

            for (int i = 0; i < (int)gMainItems.size(); i++)
            {
                RECT itemRect = { 120, 150 + i * 40, 400, 200 };

                D3DCOLOR color = (i == gSelection)
                    ? D3DCOLOR_ARGB(255, 0, 255, 255)
                    : D3DCOLOR_ARGB(255, 255, 255, 255);

                gFont->DrawTextA(
                    gSprite,
                    gMainItems[i].c_str(),
                    -1,
                    &itemRect,
                    DT_LEFT | DT_NOCLIP,
                    color
                );
            }
        }
        else if (gState == MenuState::BeatmapSelect)
        {
            RECT titleRect = { 100, 80, 800, 120 };

            gFont->DrawTextA(
                gSprite,
                "Select Beatmap",
                -1,
                &titleRect,
                DT_LEFT | DT_NOCLIP,
                D3DCOLOR_ARGB(255, 255, 255, 0)
            );

            for (int i = 0; i < (int)gBeatmaps.size(); i++)
            {
                int baseY = 150 + i * 60;

                RECT line1 = { 120, baseY, 800, baseY + 30 };
                RECT line2 = { 140, baseY + 25, 800, baseY + 50 };

                D3DCOLOR color = (i == gBeatmapSelection)
                    ? D3DCOLOR_ARGB(255, 0, 255, 255)
                    : D3DCOLOR_ARGB(255, 255, 255, 255);

                std::string mainLine =
                    gBeatmaps[i].artist + " - " +
                    gBeatmaps[i].title;

                std::string diffLine =
                    "Difficulty: " + gBeatmaps[i].difficulty;

                gFont->DrawTextA(
                    gSprite,
                    mainLine.c_str(),
                    -1,
                    &line1,
                    DT_LEFT | DT_NOCLIP,
                    color
                );

                gFont->DrawTextA(
                    gSprite,
                    diffLine.c_str(),
                    -1,
                    &line2,
                    DT_LEFT | DT_NOCLIP,
                    D3DCOLOR_ARGB(200, 180, 180, 180)
                );
            }

        }

        gSprite->End();
    }


    void OnLostDevice()
    {
        if (gFont)
            gFont->OnLostDevice();
        if (gSprite)
            gSprite->OnLostDevice();
    }

    void OnResetDevice()
    {
        if (gFont)
            gFont->OnResetDevice();
        if (gSprite)
            gSprite->OnResetDevice();
    }

}

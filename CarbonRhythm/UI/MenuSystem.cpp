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
#include <Rhythm/ScoreSystem.h>
#include <Core/Game.h>
#include <sstream>
#include <iomanip>


using json = nlohmann::json;
static LPD3DXSPRITE gSprite = nullptr;

static LPDIRECT3DTEXTURE9 gCornerTex = nullptr;

namespace
{
    static std::vector<float> gItemScale;
    static std::vector<float> gItemTargetScale;
    static std::vector<float> gBeatmapScale;
    static std::vector<float> gBeatmapTargetScale;

    static int gBeatmapScrollOffset = 0;
    static const int gMaxVisibleBeatmaps = 6;

    struct BeatmapEntry
    {
        std::string filePath;
        std::string title;
        std::string artist;
        std::string difficulty;
        Score::ScoreData score;
    };


    static std::vector<BeatmapEntry> gBeatmaps;
    static int gBeatmapSelection = 0;
	
	static float g_autoplayMsgTimer = 0.0f;  	// Auto-play message stuff
	static std::string g_autoplayMsg = "";
	static bool g_lastAutoplayState = false;
	static float g_menuDt = 0.0f;				// Auto-play message stuff

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

            std::filesystem::path p(fullPath);
            std::string id = p.stem().string();

            BeatmapEntry entry;
            entry.filePath = fullPath;
            entry.title = title;
            entry.artist = artist;
            entry.difficulty = diff;
            entry.score = Score::Load(id);

            gBeatmaps.push_back(entry);

        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);

        gBeatmapScale.resize(gBeatmaps.size(), 1.0f);
        gBeatmapTargetScale.resize(gBeatmaps.size(), 1.0f);
    }
}

namespace Menu
{
    void Initialize()
    {
        gState = MenuState::Main;
        gSelection = 0;
        gItemScale.resize(gMainItems.size(), 1.0f);
        gItemTargetScale.resize(gMainItems.size(), 1.0f);
    }

    void Update(float dt)
    {
		g_menuDt = dt;
		
        if (gItemScale.size() != gMainItems.size())
        {
            gItemScale.resize(gMainItems.size(), 1.0f);
            gItemTargetScale.resize(gMainItems.size(), 1.0f);
        }

        const auto& keys = Config::GetKeys();

        bool up = (GetAsyncKeyState(keys.MenuUp) & 0x8000) != 0;
        bool down = (GetAsyncKeyState(keys.MenuDown) & 0x8000) != 0;
        bool left = (GetAsyncKeyState(keys.MenuLeft) & 0x8000) != 0;
        bool right = (GetAsyncKeyState(keys.MenuRight) & 0x8000) != 0;

        if (gState == MenuState::Main)
        {
            for (int i = 0; i < gMainItems.size(); i++)
            {
                gItemTargetScale[i] = (i == gSelection) ? 1.25f : 1.0f;
            }

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
            if (gBeatmapScale.size() != gBeatmaps.size())
            {
                gBeatmapScale.resize(gBeatmaps.size(), 1.0f);
                gBeatmapTargetScale.resize(gBeatmaps.size(), 1.0f);
            }

            for (int i = 0; i < gBeatmaps.size(); i++)
            {
                gBeatmapTargetScale[i] = (i == gBeatmapSelection) ? 1.15f : 1.0f;
            }

            float speed = 8.0f;

            for (int i = 0; i < gBeatmaps.size(); i++)
            {
                gBeatmapScale[i] +=
                    (gBeatmapTargetScale[i] - gBeatmapScale[i]) * speed * dt;
            }

            if (up && !gPrevUp)
            {
                gBeatmapSelection--;
                if (gBeatmapSelection < 0)
                    gBeatmapSelection = gBeatmaps.size() - 1;
            }

            if (down && !gPrevDown)
            {
                gBeatmapSelection++;
                if (gBeatmapSelection >= gBeatmaps.size())
                    gBeatmapSelection = 0;
            }

            // scroll logic
            if (gBeatmapSelection < gBeatmapScrollOffset)
            {
                gBeatmapScrollOffset = gBeatmapSelection;
            }
            else if (gBeatmapSelection >= gBeatmapScrollOffset + gMaxVisibleBeatmaps)
            {
                gBeatmapScrollOffset = gBeatmapSelection - gMaxVisibleBeatmaps + 1;
            }

            if (right && !gPrevRight)
            {
                if (!gBeatmaps.empty())
                {
                    const auto& selected = gBeatmaps[gBeatmapSelection];

                    Rhythm::Initialize();   // reset dulu
                    std::filesystem::path p(selected.filePath);
                    Game::SetCurrentBeatmapId(p.stem().string());
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

        float speed = 8.0f; // tweak

        for (int i = 0; i < gMainItems.size(); i++)
        {
            float diff = gItemTargetScale[i] - gItemScale[i];
            gItemScale[i] += diff * speed * dt;

            if (fabs(diff) < 0.001f)
                gItemScale[i] = gItemTargetScale[i];
        }


    }

    void Render(IDirect3DDevice9* device)
    {
        //init bg fade
        D3DVIEWPORT9 vp;
        device->GetViewport(&vp);

        float screenW = (float)vp.Width;
        float screenH = (float)vp.Height;

        const float baseW = 1920.0f;
        const float baseH = 1080.0f;

        float scaleX = screenW / baseW;
        float scaleY = screenH / baseH;


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

        //bg
        if (!gCornerTex)
        {
            D3DXCreateTextureFromFileA(
                device,
                "CarbonRhythmAssets/CRBGFade.png",
                &gCornerTex
            );
        }


        // ===== BEGIN DRAW =====
        gSprite->Begin(D3DXSPRITE_ALPHABLEND);

        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        gSprite->SetTransform(&identity);

        D3DXVECTOR2 scaling(scaleX, scaleY);
        D3DXVECTOR2 pos(0, 0);

        D3DXMATRIX mat;
        D3DXMatrixTransformation2D(
            &mat,
            nullptr,
            0,
            &scaling,
            nullptr,
            0,
            &pos
        );

        gSprite->SetTransform(&mat);

        gSprite->Draw(
            gCornerTex,
            nullptr,
            nullptr,
            nullptr,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );

        gSprite->SetTransform(&identity);

        if (gState == MenuState::Main)
        {
            RECT titleRect = { 100, 80, 400, 120 };

            gFont->DrawTextA(
                gSprite,
                "MWRhythm v1.0.1",
                -1,
                &titleRect,
                DT_LEFT | DT_NOCLIP,
                D3DCOLOR_ARGB(255, 0, 255, 0) //A R G B
            );

            for (int i = 0; i < (int)gMainItems.size(); i++)
            {
                float scale = gItemScale[i];

                float baseX = 120.0f;
                float baseY = 150.0f + i * 50.0f;

                D3DXVECTOR2 scaling(scale, scale);
                D3DXVECTOR2 translation(baseX, baseY);

                D3DXMATRIX mat;
                D3DXMatrixTransformation2D(
                    &mat,
                    nullptr,
                    0,
                    &scaling,
                    nullptr,
                    0,
                    &translation
                );

                gSprite->SetTransform(&mat);

                RECT localRect = { 0, 0, 600, 100 };

                D3DCOLOR color = (i == gSelection)
                    ? D3DCOLOR_ARGB(255, 179, 122, 40) // 选中项 B37A28
                    : D3DCOLOR_ARGB(255, 255, 255, 255);

                gFont->DrawTextA(
                    gSprite,
                    gMainItems[i].c_str(),
                    -1,
                    &localRect,
                    DT_LEFT | DT_NOCLIP,
                    color
                );
				
            }
            gSprite->SetTransform(&identity);
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
                D3DCOLOR_ARGB(255, 0, 255, 0)
            );

            int end = min(
                gBeatmapScrollOffset + gMaxVisibleBeatmaps,
                (int)gBeatmaps.size()
            );

            for (int i = gBeatmapScrollOffset; i < end; i++)
            {
                int visibleIndex = i - gBeatmapScrollOffset;

                float scale = gBeatmapScale[i];

                float baseX = 120.0f;
                float baseY = 150.0f + visibleIndex * 110.0f;

                float itemWidth = 600.0f;
                float itemHeight = 110.0f;

                D3DXVECTOR2 center(0.0f, itemHeight * 0.5f);
                D3DXVECTOR2 scaling(scale, scale);
                D3DXVECTOR2 translation(baseX, baseY);

                D3DXMATRIX mat;
                D3DXMatrixTransformation2D(
                    &mat,
                    &center,
                    0,
                    &scaling,
                    nullptr,
                    0,
                    &translation
                );

                gSprite->SetTransform(&mat);

                RECT localRect = { 0, 0, 600, 110 };

                D3DCOLOR mainColor = (i == gBeatmapSelection)
                    ? D3DCOLOR_ARGB(255, 179, 122, 40) // 选中项 B37A28
                    : D3DCOLOR_ARGB(255, 255, 255, 255);

                std::string mainLine =
                    gBeatmaps[i].artist + " - " +
                    gBeatmaps[i].title;

                std::string diffLine =
                    "Difficulty: " + gBeatmaps[i].difficulty;

                std::string bestLine =
                    "Best: " + std::to_string(gBeatmaps[i].score.bestScore);

                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2)
                    << gBeatmaps[i].score.bestAccuracy;

                std::string statLine =
                    "Acc: " + ss.str() +
                    "% | Plays: " + std::to_string(gBeatmaps[i].score.plays);

                RECT line1 = { 0, 0, 600, 25 };
                RECT line2 = { 0, 25, 600, 50 };
                RECT line3 = { 0, 50, 600, 75 };
                RECT line4 = { 0, 75, 600, 100 };

                gFont->DrawTextA(gSprite, mainLine.c_str(), -1, &line1,
                    DT_LEFT | DT_NOCLIP, mainColor);

                gFont->DrawTextA(gSprite, diffLine.c_str(), -1, &line2,
                    DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(200, 180, 180, 180));

                gFont->DrawTextA(gSprite, bestLine.c_str(), -1, &line3,
                    DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(200, 200, 200, 255));

                gFont->DrawTextA(gSprite, statLine.c_str(), -1, &line4,
                    DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(180, 160, 160, 160));
            }

            gSprite->SetTransform(&identity);

        }
		
		// Autoplay message (works in both MainMenu and BeatmapSelect states)
		bool currentAutoplay = Rhythm::IsAutoplayEnabled();
		if (currentAutoplay != g_lastAutoplayState)
		{
			g_lastAutoplayState = currentAutoplay;
			g_autoplayMsgTimer = 2.0f;
			g_autoplayMsg = currentAutoplay ? "AUTOPLAY ON" : "AUTOPLAY OFF";
		}

		if (g_autoplayMsgTimer > 0.0f && gFont)
		{
			g_autoplayMsgTimer -= g_menuDt;
			float alpha = 1.0f;
			if (g_autoplayMsgTimer < 1.0f)
				alpha = g_autoplayMsgTimer;
			D3DCOLOR color = D3DCOLOR_ARGB((int)(alpha * 255), 0, 255, 0);
			RECT rect = { (LONG)screenW - 250, 20, (LONG)screenW, 60 };
			gSprite->SetTransform(&identity);   // identity is already defined earlier in Render()
			gFont->DrawTextA(gSprite, g_autoplayMsg.c_str(), -1, &rect, DT_LEFT | DT_NOCLIP, color);
		}
		
        gSprite->End();
    }


    void OnLostDevice()
    {
        if (gFont)
            gFont->OnLostDevice();
        if (gSprite)
            gSprite->OnLostDevice();
        if (gCornerTex)
        {
            gCornerTex->Release();
            gCornerTex = nullptr;
        }
    }

    void OnResetDevice()
    {
        if (gFont)
            gFont->OnResetDevice();
        if (gSprite)
            gSprite->OnResetDevice();
    }
}

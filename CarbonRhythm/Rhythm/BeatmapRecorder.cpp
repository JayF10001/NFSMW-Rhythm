#include "BeatmapRecorder.h"
#include <vector>
#include <fstream>
#include <Windows.h>
#include <External/json.hpp>
#include "../Rhythm/Input.h"
#include "../Audio/AudioSystem.h"
#include <Core/Config.h>
#include <Core/GameState.h>
#include <Car/DanceController.h>
#include <d3dx9.h>
#include <d3d9.h>

using json = nlohmann::json;

static LPD3DXFONT gFont = nullptr;
static LPD3DXSPRITE gSprite = nullptr;

static bool prevStart = false;
static bool prevBack = false;


namespace Recorder
{
    struct RecordedNote
    {
        double time;
        int lane;
    };

    static RecorderState gState = RecorderState::Idle;
    static std::vector<Recorder::RecordedNote> gNotes;
    static bool gRecording = false;
    static double gBPM = 120.0;
    static double gOffset = 0.0;

    RecorderState GetState()
    {
        return gState;
    }

    void EnterIdle()
    {
        static int gIdleFrames = 0;
        gIdleFrames = 2;

        gState = RecorderState::Idle;
        gRecording = false;
        // flush key
        Input::Update();
    }

    void Start(double bpm, double offset)
    {
        gNotes.clear();
        gBPM = bpm;
        gOffset = offset;
        gRecording = true;
    }

    void Stop(const std::string& outputPath)
    {
        gRecording = false;

        const auto& rec = Config::GetRecorder();

        json j;

        j["title"] = rec.Title;
        j["artist"] = rec.Artist;
        j["difficultyName"] = rec.DifficultyName;

        j["audio"] = rec.AudioPath;
        j["bpm"] = gBPM;
        j["offset"] = gOffset;

        j["difficulty"] = {
            { "perfect", rec.Difficulty.Perfect },
            { "good", rec.Difficulty.Good },
            { "bad", rec.Difficulty.Bad }
        };

        j["dance"] = {
            { "jellyBPM", rec.Dance.JellyBPM },
            { "returnSpeed", rec.Dance.ReturnSpeed },
            { "inputSpeed", rec.Dance.InputSpeed },
            { "spinLinearSpeed", rec.Dance.SpinLinearSpeed },
            { "spinEaseSpeed", rec.Dance.SpinEaseSpeed }
        };

        j["notes"] = json::array();


        int division = Config::GetRecorder().QuantizeDivision;
        double spb = 60.0 / gBPM;

        for (auto& n : gNotes)
        {
            double snapped = n.time;

            if (division > 0)
            {
                double grid = spb / division;
                snapped = round(n.time / grid) * grid;
            }

            // optional clamp precision
            snapped = floor(snapped * 1000000.0) / 1000000.0;

            j["notes"].push_back({
                { "time", snapped },
                { "lane", n.lane }
                });
        }

        std::ofstream out(outputPath);
        out << j.dump(4);
        out.close();
    }


    void Update()
    {
        if (gState == RecorderState::Idle)
        {
            const auto& keys = Config::GetKeys();
            const auto& rec = Config::GetRecorder();

            bool startHeld = (GetAsyncKeyState(rec.StartKey) & 0x8000) != 0;
            bool backHeld = (GetAsyncKeyState(rec.BackKey) & 0x8000) != 0;

            bool start = startHeld && !prevStart;
            bool back = backHeld && !prevBack;

            prevStart = startHeld;
            prevBack = backHeld;

            if (start)
            {
                Audio::Load(rec.AudioPath);
                Audio::Play();

                // APPLY DANCE CONFIG
                Dance::ApplyConfig(
                    rec.Dance.JellyBPM,
                    rec.Dance.ReturnSpeed,
                    rec.Dance.InputSpeed,
                    rec.Dance.SpinLinearSpeed,
                    rec.Dance.SpinEaseSpeed
                );

                Dance::SetEnabled(true);

                Start(rec.BPM, rec.Offset);
                gState = RecorderState::Recording;
            }

            if (back)
            {
                Game::SetState(GameState::Menu);
            }

            return;

        }

        if (gState == RecorderState::Recording)
        {
            const auto& keys = Config::GetKeys();
            const auto& rec = Config::GetRecorder();
            bool stop = (GetAsyncKeyState(rec.StopKey) & 0x8000) != 0;

            if (stop)
            {
                std::string output =
                    "CarbonRhythmAssets/maps/" +
                    Config::GetRecorder().OutputName + ".json";

                Stop(output);

                Audio::Stop();
                Dance::SetEnabled(false);

                EnterIdle();
                Game::SetState(GameState::Menu);
                return;
            }

            int pressed = Input::GetPressedMask();
            if (pressed == 0)
                return;

            double t = Audio::GetPositionSeconds();

            for (int i = 0; i < 6; i++)
            {
                if (pressed & (1 << i))
                    gNotes.push_back({ t, i });
            }
        }
    }


    bool IsRecording()
    {
        return gRecording;
    }

    void RenderInfo(IDirect3DDevice9* device)
    {

        if (!gFont)
            D3DXCreateFont(device, 20, 0, FW_BOLD, 1, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                L"Arial", &gFont);

        if (!gSprite)
            D3DXCreateSprite(device, &gSprite);

        gSprite->Begin(D3DXSPRITE_ALPHABLEND);

        RECT r = { 100, 80, 1000, 600 };

        const auto& rec = Config::GetRecorder();

        std::string text =
            "Beatmap Recorder (experimental)\n\n"
            "Please make sure your beatmap recording settings are correct (CarbonRhythm.ini):\n\n";

        text += "Audio: " + rec.AudioPath + "\n";
        text += "BPM: " + std::to_string(rec.BPM) + "\n";
        text += "Offset: " + std::to_string(rec.Offset) + "\n";
        text += "Quantize Division: 1/" +
            std::to_string(rec.QuantizeDivision) + "\n\n";

        text += "Output beatmap will be saved in:\n";
        text += "CarbonRhythmAssets\\Maps\\" + rec.OutputName + ".json\n\n";

        text += "If a file exists with the same name, it will be replaced.\n\n";
        text += "Press ENTER to start recording\n";
        text += "Press BACK to return to menu";

        gFont->DrawTextA(
            gSprite,
            text.c_str(),
            -1,
            &r,
            DT_LEFT | DT_NOCLIP,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );

        gSprite->End();
    }

    void OnLostDevice()
    {
        if (gFont) gFont->OnLostDevice();
        if (gSprite) gSprite->OnLostDevice();
    }

    void OnResetDevice()
    {
        if (gFont) gFont->OnResetDevice();
        if (gSprite) gSprite->OnResetDevice();
    }


}

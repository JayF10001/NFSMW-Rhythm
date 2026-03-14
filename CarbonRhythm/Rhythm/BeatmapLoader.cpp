#include "BeatmapLoader.h"
#include "Rhythm/RhythmSystem.h"
#include "Audio/AudioSystem.h"
#include <fstream>
#include <External/json.hpp>
#include <Windows.h>
#include "../Car/DanceController.h"


using json = nlohmann::json;

namespace Beatmap
{
    bool Load(const std::string& path)
    {
        OutputDebugStringA("Beatmap::Load called\n");

        std::ifstream file(path);
        if (!file.is_open())
        {
            OutputDebugStringA("Beatmap file failed to open\n");
            return false;
        }

        json j;
        file >> j;

        Rhythm::ClearNotes();

        double bpm = j["bpm"];
        double offset = j["offset"];
        double perfect = 0.08;
        double good = 0.1;
        double bad = 0.14;

        if (j.contains("difficulty"))
        {
            auto d = j["difficulty"];

            perfect = d.value("perfect", perfect);
            good = d.value("good", good);
            bad = d.value("bad", bad);
        }

        Rhythm::SetJudgementWindows(perfect, good, bad);

        if (j.contains("dance"))
        {
            auto d = j["dance"];

            float jellyBPM = (float)d.value("jellyBPM", bpm);
            float returnSpeed = d.value("returnSpeed", 3.0f);
            float inputSpeed = d.value("inputSpeed", 3.0f);
            float spinLinear = d.value("spinLinearSpeed", 1.0f);
            float spinEase = d.value("spinEaseSpeed", 2.0f);

            Dance::SetConfig(
                jellyBPM,
                returnSpeed,
                inputSpeed,
                spinLinear,
                spinEase
            );
        }


        Rhythm::SetBPM(bpm);
        Rhythm::SetOffset(offset);

        for (auto& note : j["notes"])
        {
            double time = note["time"];

            int mask = 0;

            if (note["lane"].is_array())
            {
                for (auto& l : note["lane"])
                    mask |= 1 << l.get<int>();
            }
            else
            {
                mask = 1 << note["lane"].get<int>();
            }

            Rhythm::AddNote(time, mask);
        }



        if (!Audio::Initialize())
        {
            OutputDebugStringA("BASS INIT FAILED\n");
            return false;
        }

        if (!Audio::Load(j["audio"]))
        {
            OutputDebugStringA("AUDIO LOAD FAILED\n");
            return false;
        }


        Audio::Play();

        return true;
    }
}

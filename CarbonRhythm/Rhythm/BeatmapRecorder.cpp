#include "BeatmapRecorder.h"
#include <vector>
#include <fstream>
#include <Windows.h>
#include <External/json.hpp>
#include "../Rhythm/Input.h"
#include "../Audio/AudioSystem.h"

using json = nlohmann::json;

namespace Recorder
{
    struct RecordedNote
    {
        double time;
        int lane;
    };

    static std::vector<RecordedNote> gNotes;
    static bool gRecording = false;
    static double gBPM = 120.0;
    static double gOffset = 0.0;

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

        json j;
        j["bpm"] = gBPM;
        j["offset"] = gOffset;

        // quantize ke 1/4 beat
        double spb = 60.0 / gBPM;
        double grid = spb / 4.0;

        for (auto& n : gNotes)
        {
            double snapped = round(n.time / grid) * grid;

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
        if (!gRecording)
            return;

        int pressed = Input::GetPressedMask();

        if (pressed == 0)
            return;

        double t = Audio::GetPositionSeconds();

        for (int i = 0; i < 6; i++)
        {
            if (pressed & (1 << i))
            {
                gNotes.push_back({ t, i });
            }
        }
    }

    bool IsRecording()
    {
        return gRecording;
    }
}

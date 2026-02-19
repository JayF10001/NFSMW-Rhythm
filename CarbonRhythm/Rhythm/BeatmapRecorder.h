#pragma once
#include <string>

namespace Recorder
{
    void Start(double bpm, double offset);
    void Stop(const std::string& outputPath);

    void Update();

    bool IsRecording();
}

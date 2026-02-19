#pragma once
#include <string>

namespace Audio
{
    bool Initialize();
    bool Load(const std::string& path);
    void Play();
    void Stop();
    double GetPositionSeconds();
    bool IsFinished();
}

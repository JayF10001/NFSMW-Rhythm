#pragma once
#include <string>

namespace Game
{
    void Initialize();
    void SetCurrentBeatmapId(const std::string& id);
    const std::string& GetCurrentBeatmapId();
}

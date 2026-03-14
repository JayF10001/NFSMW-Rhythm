#pragma once
#include <string>

namespace Score
{
    struct ScoreData
    {
        int bestScore = 0;
        int bestCombo = 0;
        double bestAccuracy = 0.0;
        int plays = 0;

        int lastScore = 0;
        int lastCombo = 0;
        double lastAccuracy = 0.0;
        std::string lastDate;
    };

    void Save(const std::string& beatmapId,
        int score,
        int combo,
        double accuracy);

    ScoreData Load(const std::string& beatmapId);
}

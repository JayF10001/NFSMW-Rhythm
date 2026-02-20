#include "ScoreSystem.h"
#include <fstream>
#include <filesystem>
#include <ctime>
#include <External/json.hpp>

using json = nlohmann::json;

namespace
{
    std::string GetScorePath(const std::string& id)
    {
        std::filesystem::create_directories("CarbonRhythmAssets/scores/");
        return "CarbonRhythmAssets/scores/" + id + ".json";
    }

    std::string GetCurrentDate()
    {
        time_t now = time(nullptr);
        tm local;
        localtime_s(&local, &now);

        char buffer[64];
        strftime(buffer, sizeof(buffer),
            "%Y-%m-%d %H:%M", &local);

        return buffer;
    }
}

namespace Score
{
    ScoreData Load(const std::string& beatmapId)
    {
        ScoreData data;

        std::ifstream file(GetScorePath(beatmapId));
        if (!file.is_open())
            return data;

        json j;
        file >> j;

        data.bestScore = j.value("bestScore", 0);
        data.bestCombo = j.value("bestCombo", 0);
        data.bestAccuracy = j.value("bestAccuracy", 0.0);
        data.plays = j.value("plays", 0);

        if (j.contains("lastResult"))
        {
            auto& lr = j["lastResult"];
            data.lastScore = lr.value("score", 0);
            data.lastCombo = lr.value("combo", 0);
            data.lastAccuracy = lr.value("accuracy", 0.0);
            data.lastDate = lr.value("date", "");
        }

        return data;
    }

    void Save(const std::string& beatmapId,
        int score,
        int combo,
        double accuracy)
    {
        ScoreData data = Load(beatmapId);

        data.plays++;

        if (score > data.bestScore)
            data.bestScore = score;

        if (combo > data.bestCombo)
            data.bestCombo = combo;

        if (accuracy > data.bestAccuracy)
            data.bestAccuracy = accuracy;

        data.lastScore = score;
        data.lastCombo = combo;
        data.lastAccuracy = accuracy;
        data.lastDate = GetCurrentDate();

        json j;
        j["bestScore"] = data.bestScore;
        j["bestCombo"] = data.bestCombo;
        j["bestAccuracy"] = data.bestAccuracy;
        j["plays"] = data.plays;

        j["lastResult"] =
        {
            {"score", data.lastScore},
            {"combo", data.lastCombo},
            {"accuracy", data.lastAccuracy},
            {"date", data.lastDate}
        };

        std::ofstream out(GetScorePath(beatmapId));
        out << j.dump(4);
    }
}

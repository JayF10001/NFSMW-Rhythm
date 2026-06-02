#pragma once
#include <string>

namespace Config
{
    struct KeyBindings
    {
        // ===== MENU =====
        int MenuToggle;
        int MenuUp;
        int MenuDown;
        int MenuLeft;
        int MenuRight;
        int AutoplayToggle;

        // ===== RECORDER =====
        int StartRecording;
        int StopRecording;

        // ===== GAME LANES =====
        int Lane[6];   // 0 = slow spin, 1 = left, dst
    };

    struct RecorderSettings
    {
        struct DifficultySettings
        {
            double Perfect = 0.03;
            double Good = 0.06;
            double Bad = 0.09;
        };

        struct DanceSettings
        {
            double JellyBPM = 120.0;
            double ReturnSpeed = 4.1;
            double InputSpeed = 5.0;
            double SpinLinearSpeed = 0.3;
            double SpinEaseSpeed = 2.0;
            bool SyncJellyToCurrentBPM = true;
        };


        DifficultySettings Difficulty;
        DanceSettings Dance;

        std::string AudioPath;
        std::string OutputName;
        double BPM = 120.0;
        double Offset = 0.0;
        int QuantizeDivision = 4;

        int StartKey;
        int BackKey;
        int StopKey;

        std::string Title;
        std::string Artist;
        std::string DifficultyName;
    };


    void Load();
    void Reload();

    const KeyBindings& GetKeys();
    const RecorderSettings& GetRecorder();
}

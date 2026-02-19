#include "Config.h"
#include <Windows.h>
#include <cstdlib>
#include <filesystem>

namespace
{
    Config::KeyBindings gKeys;
    Config::RecorderSettings gRecorder;

    std::string gIniPath;

    int ReadInt(const char* section, const char* key, int def)
    {
        return GetPrivateProfileIntA(section, key, def, gIniPath.c_str());
    }

    double ReadDouble(const char* section, const char* key, double def)
    {
        char buffer[64];
        GetPrivateProfileStringA(section, key, "", buffer, sizeof(buffer), gIniPath.c_str());

        if (buffer[0] == 0)
            return def;

        return atof(buffer);
    }

    std::string ReadString(const char* section, const char* key, const char* def)
    {
        char buffer[260];
        GetPrivateProfileStringA(section, key, def, buffer, sizeof(buffer), gIniPath.c_str());
        return std::string(buffer);
    }

    void BuildIniPath()
    {
        char path[MAX_PATH];

        HMODULE hModule = NULL;
        GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            (LPCSTR)&BuildIniPath,
            &hModule
        );

        GetModuleFileNameA(hModule, path, MAX_PATH);

        // potong sampai folder
        char* p = strrchr(path, '\\');
        if (p) *(p + 1) = 0;

        strcat_s(path, "CarbonRhythm.ini");

        gIniPath = path;
    }

}

namespace Config
{
    void Load()
    {
        BuildIniPath();

        // ===== LANE CONTROLS =====
        gKeys.Lane[0] = ReadInt("Controls", "SlowSpin", 'U');
        gKeys.Lane[1] = ReadInt("Controls", "Left", 'J');
        gKeys.Lane[2] = ReadInt("Controls", "Down", 'K');
        gKeys.Lane[3] = ReadInt("Controls", "Up", 'I');
        gKeys.Lane[4] = ReadInt("Controls", "Right", 'L');
        gKeys.Lane[5] = ReadInt("Controls", "QuickSpin", 'O');

        // ===== RECORDER KEYS =====
        gKeys.StartRecording = ReadInt("Keybinds", "StartRecording", VK_F3);
        gKeys.StopRecording = ReadInt("Keybinds", "StopRecording", VK_F4);

        // ===== KEYBINDINGS =====
        gKeys.MenuToggle = ReadInt("Keybinds", "MenuToggle", VK_F2);
        gKeys.MenuUp = ReadInt("Keybinds", "MenuUp", VK_UP);
        gKeys.MenuDown = ReadInt("Keybinds", "MenuDown", VK_DOWN);
        gKeys.MenuLeft = ReadInt("Keybinds", "MenuLeft", VK_LEFT);
        gKeys.MenuRight = ReadInt("Keybinds", "MenuRight", VK_RIGHT);

        // ===== RECORDER SETTINGS =====
        gRecorder.AudioPath = ReadString("Recorder", "AudioPath", "song.mp3");
        gRecorder.OutputName = ReadString("Recorder", "OutputName", "output");
        gRecorder.BPM = ReadDouble("Recorder", "BPM", 120.0);
        gRecorder.Offset = ReadDouble("Recorder", "Offset", -0.05);
        gRecorder.QuantizeDivision = ReadInt("Recorder", "QuantizeDivision", 4);
        gRecorder.StartKey = ReadInt("Recorder", "StartKey", 13);
        gRecorder.BackKey = ReadInt("Recorder", "BackKey", 8);
        gRecorder.StopKey = ReadInt("Recorder", "StopKey", 115);
        gRecorder.Difficulty.Perfect =
            ReadDouble("Difficulty", "Perfect", 0.03);
        gRecorder.Difficulty.Good =
            ReadDouble("Difficulty", "Good", 0.06);
        gRecorder.Difficulty.Bad =
            ReadDouble("Difficulty", "Bad", 0.09);
        gRecorder.Dance.JellyBPM =
            ReadDouble("Dance", "JellyBPM", gRecorder.BPM);
        gRecorder.Dance.ReturnSpeed =
            ReadDouble("Dance", "ReturnSpeed", 4.1);
        gRecorder.Dance.InputSpeed =
            ReadDouble("Dance", "InputSpeed", 5.0);
        gRecorder.Dance.SpinLinearSpeed =
            ReadDouble("Dance", "SpinLinearSpeed", 0.3);
        gRecorder.Dance.SpinEaseSpeed =
            ReadDouble("Dance", "SpinEaseSpeed", 2.0);
    }

    void Reload()
    {
        Load();
    }

    const KeyBindings& GetKeys()
    {
        return gKeys;
    }

    const RecorderSettings& GetRecorder()
    {
        return gRecorder;
    }
}

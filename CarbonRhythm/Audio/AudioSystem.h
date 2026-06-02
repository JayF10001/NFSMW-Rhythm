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
	void PlayOnce(const std::string& path, float volume = 1.0f); //Code needed to play Hit-Sounds
}
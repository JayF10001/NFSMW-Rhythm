#pragma once

#include <string>
#include <d3d9.h>

namespace Recorder
{
    enum class RecorderState
    {
        Idle,
        Recording
    };

    void EnterIdle();
    void Start(double bpm, double offset);
    void Stop(const std::string& outputPath);
    void Update();

    RecorderState GetState();

    void RenderInfo(IDirect3DDevice9* device);
    void OnLostDevice();
    void OnResetDevice();
}

#pragma once
#include <d3d9.h>

namespace Overlay
{
    void Render(IDirect3DDevice9* device);
    void OnLostDevice();
    void OnResetDevice();
}
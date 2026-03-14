#pragma once
#include <d3d9.h>

namespace Menu
{
    void Initialize();
    void Toggle();

    void Update(float dt);
    void Render(IDirect3DDevice9* device);

    bool IsOpen();

    void OnResetDevice();
    void OnLostDevice();
}

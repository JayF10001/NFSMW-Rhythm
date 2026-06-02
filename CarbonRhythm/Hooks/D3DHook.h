#pragma once
#include <d3d9.h>

namespace D3DHook
{
    void Initialize();
    void OnEndScene(IDirect3DDevice9* device);
}
#pragma once

namespace Input
{
    void Update();
    int GetPressedMask();
    int GetHeldMask();
    void SetKey(int lane, int vk);
}

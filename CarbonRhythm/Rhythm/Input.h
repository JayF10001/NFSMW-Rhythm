#pragma once

namespace Input
{
    void Update();
    int GetPressedMask();
    int GetHeldMask(); //This is needed if you want to add detecting pressing more than 1 key.
	int GetVisualHeldMask();
    void SetKey(int lane, int vk);
}
#include <Windows.h>
#include "Input.h"

namespace Input
{
    static int currentMask = 0;
    static int prevMask = 0;
    static int downMask = 0;

    static int gKeyCodes[6] = { 85, 74, 75, 73, 76, 79 };


    void Update()
    {
        currentMask = 0;

        for (int i = 0; i < 6; i++)
        {
            if (GetAsyncKeyState(gKeyCodes[i]) & 0x8000)
                currentMask |= 1 << i;
        }

        downMask = currentMask & (~prevMask);
        prevMask = currentMask;
    }


    int GetPressedMask()
    {
        return downMask;
    }

    int GetHeldMask()
    {
        return currentMask;
    }

    void SetKey(int lane, int vk)
    {
        if (lane >= 0 && lane < 6)
            gKeyCodes[lane] = vk;
    }


}

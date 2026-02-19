#include "GameTime.h"

uint64_t bGetTicker()
{
    return ((uint64_t(__cdecl*)())0x46CEC0)();
}

float& TicksToMilliseconds = *(float*)0xA84A10;

#pragma once
#include <cstdint>

namespace GameTime
{
    // Monotonic seconds since first use of GameTime (process/module lifetime).
    double NowSeconds();

    // Computes frame delta time in seconds. The caller owns lastCounter storage.
    // dt is clamped to [0.0f, 0.1f] to avoid large jumps after stalls.
    float DeltaSeconds(uint64_t& lastCounter);
}

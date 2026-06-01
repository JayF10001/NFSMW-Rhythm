#include "GameTime.h"
#include <Windows.h>

namespace
{
    struct ClockState
    {
        bool initialized = false;
        bool useQpc = false;
        uint64_t frequency = 1000;
        uint64_t startCounter = 0;
    };

    ClockState& GetClockState()
    {
        static ClockState state;
        if (!state.initialized)
        {
            LARGE_INTEGER freq{};
            if (QueryPerformanceFrequency(&freq) && freq.QuadPart > 0)
            {
                state.useQpc = true;
                state.frequency = static_cast<uint64_t>(freq.QuadPart);

                LARGE_INTEGER now{};
                QueryPerformanceCounter(&now);
                state.startCounter = static_cast<uint64_t>(now.QuadPart);
            }
            else
            {
                state.useQpc = false;
                state.frequency = 1000;
                state.startCounter = GetTickCount64();
            }

            state.initialized = true;
        }
        return state;
    }

    uint64_t ReadCounter()
    {
        const auto& state = GetClockState();
        if (state.useQpc)
        {
            LARGE_INTEGER now{};
            QueryPerformanceCounter(&now);
            return static_cast<uint64_t>(now.QuadPart);
        }
        return GetTickCount64();
    }

    float ClampDt(float dt)
    {
        if (dt < 0.0f) return 0.0f;
        if (dt > 0.1f) return 0.1f;
        return dt;
    }
}

double GameTime::NowSeconds()
{
    const auto& state = GetClockState();
    const uint64_t now = ReadCounter();
    if (now < state.startCounter)
        return 0.0;
    return static_cast<double>(now - state.startCounter) / static_cast<double>(state.frequency);
}

float GameTime::DeltaSeconds(uint64_t& lastCounter)
{
    const auto& state = GetClockState();
    const uint64_t now = ReadCounter();

    if (lastCounter == 0)
    {
        lastCounter = now;
        return 0.0f;
    }

    if (now < lastCounter)
    {
        lastCounter = now;
        return 0.0f;
    }

    const uint64_t delta = now - lastCounter;
    lastCounter = now;

    const float dt = static_cast<float>(
        static_cast<double>(delta) / static_cast<double>(state.frequency)
    );

    return ClampDt(dt);
}

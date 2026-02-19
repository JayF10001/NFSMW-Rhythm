#pragma once

namespace Dance
{
    void SetConfig(
        float jellyBPM,
        float returnSpeed,
        float inputSpeed,
        float spinLinearSpeed,
        float spinEaseSpeed
    );

    void ApplyConfig(
        double jellyBPM,
        double returnSpeed,
        double inputSpeed,
        double spinLinearSpeed,
        double spinEaseSpeed
    );

    void Initialize();
    void Update(float dt);
    void Toggle();
    bool IsEnabled();
    void SetEnabled(bool state);
}

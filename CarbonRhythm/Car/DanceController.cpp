#include "DanceController.h"
#include <Windows.h>
#include <cmath>
#include "../Rhythm/Input.h"
#include "../Rhythm/RhythmSystem.h"


struct Matrix4
{
    struct { float x, y, z, w; } x, y, z, p;
};

// auto& CarMatrix = *(Matrix4*)0xB778D0; // nfsc: 0xB778D0
auto& CarMatrix = *(Matrix4*)0x9B34B0; // nfsmw: 0x9B34B0

static bool gEnabled = false;
static bool gPrevF2 = false;

static float gRotX = 0.0f;
static float gRotY = 0.0f;
static float gRotZ = 0.0f;

static float gJellyBPM = 120.0f;
static float gReturnSpeed = 4.0f;
static float gInputSpeed = 5.0f;
static float gSpinLinearSpeed = 1.0f;
static float gSpinEaseSpeed = 2.0f;

static float gJellyState = 0.0f;
static bool  gJellyBack = false;
static float gJellySpeed = 4.0f;

static bool  gSpinLinearActive = false;
static bool  gSpinEaseActive = false;
static float gSpinAngle = 0.0f;
static float gSpinProgress = 0.0f;


static float EaseInOutExpo(float t)
{
    if (t == 0.0f)
        return 0.0f;

    if (t == 1.0f)
        return 1.0f;

    if (t < 0.5f)
        return powf(2.0f, 20.0f * t - 10.0f) / 2.0f;
    else
        return (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

static JellyEaseType gJellyEase = JellyEaseType::EaseOut;

namespace Dance
{
    void Toggle()
    {
        gEnabled = !gEnabled;

        if (!gEnabled)
        {
            gRotX = gRotY = gRotZ = 0.0f;

            gSpinLinearActive = false;
            gSpinEaseActive = false;
            gSpinAngle = 0.0f;
            gSpinProgress = 0.0f;

            gJellyState = 0.0f;
            gJellyBack = false;
        }

    }

    bool IsEnabled()
    {
        return gEnabled;
    }

    void Initialize()
    {
        gRotX = gRotY = gRotZ = 0.0f;
    }

    static void ApplyMatrix()
    {
        CarMatrix.x = { 1,0,0,0 };
        CarMatrix.y = { 0,1,0,0 };
        CarMatrix.z = { 0,0,1,0 };
        CarMatrix.p = { 0,0,0,1 };

        float cx = cosf(gRotX);
        float sx = sinf(gRotX);
        float cy = cosf(gRotY);
        float sy = sinf(gRotY);
        float cz = cosf(gRotZ);
        float sz = sinf(gRotZ);

        CarMatrix.x.x = cy * cz;
        CarMatrix.x.y = -cy * sz;
        CarMatrix.x.z = sy;

        CarMatrix.y.x = sx * sy * cz + cx * sz;
        CarMatrix.y.y = -sx * sy * sz + cx * cz;
        CarMatrix.y.z = -sx * cy;

        CarMatrix.z.x = -cx * sy * cz + sx * sz;
        CarMatrix.z.y = cx * sy * sz + sx * cz;
        CarMatrix.z.z = cx * cy;

        float horiz = EaseInOutExpo(1.0f - gJellyState);
        float vert = EaseInOutExpo(gJellyState);

        CarMatrix.x.x *= 1.0f + horiz * 0.3f;
        CarMatrix.y.y *= 1.0f + horiz * 0.3f;
        CarMatrix.z.z *= 1.0f + vert * 0.3f;

    }

    static void UpdateJelly(float dt)
    {
        float speed = gJellySpeed * dt;

        gJellyState += gJellyBack ? -speed : speed;

        if (gJellyState > 1.0f)
        {
            gJellyState = 1.0f;
            gJellyBack = true;
        }

        if (gJellyState < 0.0f)
        {
            gJellyState = 0.0f;
            gJellyBack = false;
        }
    }


    void Update(float dt)
    {
        if (!gEnabled)
            return;

        float speed = gInputSpeed * dt;

        int held = Input::GetHeldMask();

        if (held & (1 << 1)) gRotX += speed;  // J
        if (held & (1 << 4)) gRotX -= speed;  // L
        if (held & (1 << 3)) gRotY += speed;  // I
        if (held & (1 << 2)) gRotY -= speed;  // K

        int pressed = Input::GetPressedMask();

        // slow spin trigger (lane 0) - restart or extend
		if (pressed & (1 << 0))
		{
			if (!gSpinLinearActive)
			{
				gSpinLinearActive = true;
				gSpinAngle = 0.0f;
			}
			else
			{
				// restart current spin to add another full rotation
				gSpinAngle = 0.0f;
			}
		}

		// quick spin trigger (lane 5) - restart or extend
		if (pressed & (1 << 5))
		{
			if (!gSpinEaseActive)
			{
				gSpinEaseActive = true;
				gSpinProgress = 0.0f;
			}
			else
			{
				// reset progress to start another quick spin
				gSpinProgress = 0.0f;
			}
		}

        // LINEAR SPIN
        if (gSpinLinearActive)
        {
            gSpinAngle += gSpinLinearSpeed * dt * 6.28318f;
            gRotZ = gSpinAngle;

            if (gSpinAngle >= 6.28318f)
            {
                gSpinLinearActive = false;
                gSpinAngle = 0.0f;
                gRotZ = 0.0f;
            }
        }

        // EASE SPIN
        if (gSpinEaseActive)
        {
            gSpinProgress += gSpinEaseSpeed * dt;

            float t = gSpinProgress;
            if (t > 1.0f) t = 1.0f;

            float eased = (t < 0.5f)
                ? 4.0f * t * t * t
                : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;

            gRotZ = eased * 6.28318f;

            if (gSpinProgress >= 1.0f)
            {
                gSpinEaseActive = false;
                gSpinProgress = 0.0f;
                gRotZ = 0.0f;
            }
        }

        if (!gSpinLinearActive && !gSpinEaseActive)
            gRotZ += (0 - gRotZ) * gReturnSpeed * dt;


        // auto return
        gRotX += (0 - gRotX) * gReturnSpeed * dt;
        gRotY += (0 - gRotY) * gReturnSpeed * dt;

        UpdateJelly(dt);
        ApplyMatrix();
    }

    void SetConfig(

        float jellyBPM,
        float returnSpeed,
        float inputSpeed,
        float spinLinearSpeed,
        float spinEaseSpeed)
    {
        gJellyBPM = jellyBPM;
        gReturnSpeed = returnSpeed;
        gInputSpeed = inputSpeed;
        gSpinLinearSpeed = spinLinearSpeed;
        gSpinEaseSpeed = spinEaseSpeed;

        gJellySpeed = gJellyBPM / 30.0f;
    }

    void SetEnabled(bool state)
    {
        gEnabled = state;

        if (!gEnabled)
        {
            gRotX = gRotY = gRotZ = 0.0f;

            gSpinLinearActive = false;
            gSpinEaseActive = false;
            gSpinAngle = 0.0f;
            gSpinProgress = 0.0f;

            gJellyState = 0.0f;
            gJellyBack = false;

            // RESET MATRIX
            CarMatrix.x = { 1,0,0,0 };
            CarMatrix.y = { 0,1,0,0 };
            CarMatrix.z = { 0,0,1,0 };
            CarMatrix.p = { 0,0,0,1 };
        }

    }

    void ApplyConfig(
        double jellyBPM,
        double returnSpeed,
        double inputSpeed,
        double spinLinearSpeed,
        double spinEaseSpeed)
    {
        gJellyBPM = jellyBPM;
        gReturnSpeed = returnSpeed;
        gInputSpeed = inputSpeed;
        gSpinLinearSpeed = spinLinearSpeed;
        gSpinEaseSpeed = spinEaseSpeed;
    }

    void SetJellyEase(JellyEaseType type)
    {
        gJellyEase = type;
    }

    static float ApplyEase(float t)
{
    switch (gJellyEase)
    {
    case JellyEaseType::Linear:
        return t;

    case JellyEaseType::EaseOut:
        return 1.0f - powf(1.0f - t, 3.0f);

    case JellyEaseType::EaseInOut:
        if (t < 0.5f)
            return 4 * t * t * t;
        else
            return 1 - powf(-2 * t + 2, 3) / 2;

    case JellyEaseType::Elastic:
        return sinf(-13.0f * (t + 1) * 3.14159f / 2) * powf(2.0f, -10.0f * t) + 1;

    case JellyEaseType::Bounce:
        if (t < 0.3636f)
            return 7.5625f * t * t;
        else if (t < 0.7272f)
        {
            t -= 0.5454f;
            return 7.5625f * t * t + 0.75f;
        }
        else if (t < 0.9090f)
        {
            t -= 0.8181f;
            return 7.5625f * t * t + 0.9375f;
        }
        else
        {
            t -= 0.9545f;
            return 7.5625f * t * t + 0.984375f;
        }
    }

    return t;
}

void Dance::SetJellyBPM(float bpm)
{
    gJellyBPM = bpm;
    gJellySpeed = gJellyBPM / 30.0f;
}

}
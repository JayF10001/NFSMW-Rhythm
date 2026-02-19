#include "Render/Overlay.h"
#include "Hooks/D3DHook.h"
#include "Rhythm/RhythmSystem.h"
#include <cmath>
#include <d3dx9.h>
#include <d3d9.h>
#include <stdio.h>
#include <Rhythm/Input.h>

static LPD3DXFONT gFont = nullptr;
static LPDIRECT3DTEXTURE9 gArrowTex[6] = { nullptr };
static LPD3DXSPRITE gSprite = nullptr;


void Overlay::Render(IDirect3DDevice9* device)
{
    if (!device)
        return;

    if (device->TestCooperativeLevel() != D3D_OK)
        return;

    double currentTime = Rhythm::GetTime();
    double secondsPerBeat = Rhythm::GetSecondsPerBeat();
    double visibleRange = secondsPerBeat * 4.0; // tampilkan 4 beat ke depan

    if (!gFont)
    {
        D3DXCreateFont(
            device,
            18, 0,
            FW_BOLD,
            1,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"Arial",
            &gFont
        );
    }

    if (!gSprite)
    {
        D3DXCreateSprite(device, &gSprite);

        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinSlow.png", &gArrowTex[0]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowLeft.png", &gArrowTex[1]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowDown.png", &gArrowTex[2]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowUp.png", &gArrowTex[3]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowRight.png", &gArrowTex[4]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinQuick.png", &gArrowTex[5]);
    }

    float phase = Rhythm::GetBeatPhase();
    bool window = Rhythm::IsWindowActive();

    int barWidth = 300;
    int filledWidth = (int)(barWidth * phase);

    int x = 100;
    int y = 100;

    //lane
    int laneWidth = 110;
    int laneHeight = 700;
    int startX = 80;   // geser ke kiri
    int startY = 60;

    int heldMask = Input::GetHeldMask();

    for (int i = 0; i < 6; i++)
    {
        bool isHeld = (heldMask & (1 << i)) != 0;

        D3DCOLOR laneColors[6] =
        {
            D3DCOLOR_ARGB(180, 255, 100, 100),
            D3DCOLOR_ARGB(180, 255, 180, 100),
            D3DCOLOR_ARGB(180, 100, 255, 100),
            D3DCOLOR_ARGB(180, 100, 200, 255),
            D3DCOLOR_ARGB(180, 200, 100, 255),
            D3DCOLOR_ARGB(180, 255, 100, 200)
        };

        D3DCOLOR laneColor = isHeld
            ? laneColors[i]
            : D3DCOLOR_ARGB(120, 40, 40, 40);

        D3DRECT lane =
        {
            startX + i * laneWidth,
            startY,
            startX + (i + 1) * laneWidth - 5,
            startY + laneHeight
        };

        device->Clear(1, &lane,
            D3DCLEAR_TARGET,
            laneColor,
            0.0f,
            0);
    }

    D3DRECT hitLine =
    {
        startX,
        startY + laneHeight - 20,
        startX + 6 * laneWidth,
        startY + laneHeight
    };

    gSprite->Begin(D3DXSPRITE_ALPHABLEND);

    for (int i = 0; i < 6; i++)
    {
        if (!gArrowTex[i]) continue;

        D3DSURFACE_DESC desc;
        gArrowTex[i]->GetLevelDesc(0, &desc);

        float targetSize = 100.0f; // <<< ukuran arrow lo (ubah sesuka hati)
        float scale = targetSize / desc.Width;

        D3DXVECTOR2 scaling(scale, scale);
        D3DXVECTOR2 translation(
            (float)(startX + i * laneWidth + laneWidth / 2),
            (float)(startY + laneHeight - 40)
        );

        D3DXMATRIX mat;
        D3DXMatrixTransformation2D(
            &mat,
            NULL,
            0.0f,
            &scaling,
            NULL,
            0.0f,
            &translation
        );

        gSprite->SetTransform(&mat);

        D3DXVECTOR3 center(desc.Width / 2.0f, desc.Height / 2.0f, 0);

        gSprite->Draw(
            gArrowTex[i],
            NULL,
            &center,
            NULL,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );

    }


    device->Clear(1, &hitLine, D3DCLEAR_TARGET,
        D3DCOLOR_ARGB(200, 255, 255, 255),
        0.0f,
        0);

    

    // 3️⃣ NOTE MARKER (baru)
    int hitLineY = startY + laneHeight - 80;

    float pixelsPerSecond = 500.0f; // scroll speed

    for (int i = 0; i < 16; i++)
    {
        int index = Rhythm::GetCurrentNoteIndex() + i;

        if (!Rhythm::IsNoteValid(index))
            break;

        double noteTime = Rhythm::GetNoteTime(index);
        double timeDiff = noteTime - currentTime;

        // jangan render note yang udah lewat
        if (timeDiff < -0.2)
            continue;

        // posisi Y (jatuh ke bawah)
        int noteY = hitLineY - (int)(timeDiff * pixelsPerSecond);

        // kalau di luar layar
        if (noteY < startY - 100)
            continue;
        if (noteY > startY + laneHeight)
            continue;


        // ambil lane dari keyMask
        int laneIndex = 0;
        int mask = Rhythm::GetNoteKeyMask(index);

        for (int k = 0; k < 6; k++)
        {
            if (mask & (1 << k))
            {
                laneIndex = k;
                break;
            }
        }

        int laneX = startX + laneIndex * laneWidth;

        float noteCenterX = (float)(laneX + laneWidth / 2);
        float noteCenterY = (float)noteY;

        D3DSURFACE_DESC texDesc;
        gArrowTex[laneIndex]->GetLevelDesc(0, &texDesc);

        float targetSize = 100.0f; // ukuran note gede
        float scale = targetSize / texDesc.Width;

        D3DXVECTOR2 scaling(scale, scale);
        D3DXVECTOR2 translation(noteCenterX, noteCenterY);

        D3DXMATRIX mat;
        D3DXMatrixTransformation2D(
            &mat,
            NULL,
            0.0f,
            &scaling,
            NULL,
            0.0f,
            &translation
        );

        gSprite->SetTransform(&mat);

        D3DXVECTOR3 center(texDesc.Width / 2.0f, texDesc.Height / 2.0f, 0);

        gSprite->Draw(
            gArrowTex[laneIndex],
            NULL,
            &center,
            NULL,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );

    }

    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    gSprite->SetTransform(&identity);

    gSprite->End();


    // 5️⃣ Score text (paling akhir)
    if (gFont)
    {
        char buffer[128];
        sprintf_s(buffer, "Score: %d\nCombo: %d",
            Rhythm::GetScore(),
            Rhythm::GetCombo());

        RECT textRect = { 100, 200, 400, 300 };

        gFont->DrawTextA(
            NULL,
            buffer,
            -1,
            &textRect,
            DT_LEFT,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );

        const char* judgeText = "";

        switch (Rhythm::GetLastJudgement())
        {
        case Rhythm::Judgement::Perfect: judgeText = "PERFECT"; break;
        case Rhythm::Judgement::Good:    judgeText = "GOOD"; break;
        case Rhythm::Judgement::Bad:     judgeText = "BAD"; break;
        case Rhythm::Judgement::Miss:    judgeText = "MISS"; break;
        }

        RECT judgeRect = { 100, 250, 400, 350 };

        gFont->DrawTextA(
            NULL,
            judgeText,
            -1,
            &judgeRect,
            DT_LEFT,
            D3DCOLOR_ARGB(255, 255, 255, 0)
        );

    }   
}

void Overlay::OnLostDevice()
{
    if (gSprite) gSprite->OnLostDevice();
    if (gFont) gFont->OnLostDevice();
}

void Overlay::OnResetDevice()
{
    if (gSprite) gSprite->OnResetDevice();
    if (gFont) gFont->OnResetDevice();
}

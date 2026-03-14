#include "Render/Overlay.h"
#include "Hooks/D3DHook.h"
#include "Rhythm/RhythmSystem.h"
#include <cmath>
#include <d3dx9.h>
#include <d3d9.h>
#include <stdio.h>
#include <Rhythm/Input.h>
#include <string>


static LPD3DXFONT gFont = nullptr;
static LPDIRECT3DTEXTURE9 gArrowTex[6] = { nullptr };
static LPDIRECT3DTEXTURE9 gArrowTexL[6] = { nullptr };
static LPDIRECT3DTEXTURE9 gCornerTex = nullptr;
static LPD3DXSPRITE gSprite = nullptr;
static LPDIRECT3DTEXTURE9 gJudgeTex[4] = { nullptr };
// 0=Perfect, 1=Good, 2=Bad, 3=Miss

static LPDIRECT3DTEXTURE9 gDigitTex[11] = { nullptr };
// 0-9 = angka, 10 = X

static LPDIRECT3DTEXTURE9 gOverlayTex = nullptr;

static LPDIRECT3DTEXTURE9 gArrowFlashTex[6] = { nullptr };

const float SCORE_SCALE = 0.18f;
const float COMBO_SCALE = 0.21f;
const float COMBO_X_RATIO = 0.8f;

static float gLaneFlashInput[6] = { 0 };
static float gNoteHitFlash[6] = { 0 };

void Overlay::Render(IDirect3DDevice9* device)
{
    if (!device)
        return;

    if (device->TestCooperativeLevel() != D3D_OK)
        return;

    double currentTime = Rhythm::GetTime();
    double secondsPerBeat = Rhythm::GetSecondsPerBeat();
    double visibleRange = secondsPerBeat * 4.0; // tampilkan 4 beat ke depan

    D3DVIEWPORT9 vp;
    device->GetViewport(&vp);

    float screenW = (float)vp.Width;
    float screenH = (float)vp.Height;

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
    }

    if (!gArrowTex[0])
    {
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinSlow.png", &gArrowTex[0]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowLeft.png", &gArrowTex[1]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowDown.png", &gArrowTex[2]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowUp.png", &gArrowTex[3]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowRight.png", &gArrowTex[4]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinQuick.png", &gArrowTex[5]);

        //land
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinSlow_Land.png", &gArrowTexL[0]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowLeft_Land.png", &gArrowTexL[1]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowDown_Land.png", &gArrowTexL[2]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowUp_Land.png", &gArrowTexL[3]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowRight_Land.png", &gArrowTexL[4]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinQuick_Land.png", &gArrowTexL[5]);

        // flash notes
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinSlow_Flash.png", &gArrowFlashTex[0]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowLeft_Flash.png", &gArrowFlashTex[1]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowDown_Flash.png", &gArrowFlashTex[2]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowUp_Flash.png", &gArrowFlashTex[3]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRArrowRight_Flash.png", &gArrowFlashTex[4]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRSpinQuick_Flash.png", &gArrowFlashTex[5]);

        // judgement
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRJudgePerfect.png", &gJudgeTex[0]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRJudgeGood.png", &gJudgeTex[1]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRJudgeBad.png", &gJudgeTex[2]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRJudgeMiss.png", &gJudgeTex[3]);

        // digits
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum0.png", &gDigitTex[0]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum1.png", &gDigitTex[1]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum2.png", &gDigitTex[2]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum3.png", &gDigitTex[3]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum4.png", &gDigitTex[4]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum5.png", &gDigitTex[5]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum6.png", &gDigitTex[6]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum7.png", &gDigitTex[7]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum8.png", &gDigitTex[8]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNum9.png", &gDigitTex[9]);
        D3DXCreateTextureFromFile(device, L"CarbonRhythmAssets/CRNumX.png", &gDigitTex[10]);
    }

    if (!gCornerTex)
    {
        D3DXCreateTextureFromFileA(
            device,
            "CarbonRhythmAssets/CRBGFade.png",
            &gCornerTex
        );
    }

    if (!gOverlayTex)
    {
        HRESULT hr = D3DXCreateTextureFromFile(
            device,
            L"CarbonRhythmAssets/CRBGFadeBottom.png",
            &gOverlayTex
        );
    }

    float phase = Rhythm::GetBeatPhase();
    bool window = Rhythm::IsWindowActive();

    int barWidth = 300;
    int filledWidth = (int)(barWidth * phase);

    int x = 100;
    int y = 100;

    int heldMask = Input::GetHeldMask();

    for (int i = 0; i < 6; i++)
    {
        if (heldMask & (1 << i))
            gLaneFlashInput[i] = 0.08f;
    }

    float dt = Rhythm::GetDeltaTime();

    for (int i = 0; i < 6; i++)
    {
        if (gLaneFlashInput[i] > 0.0f)
            gLaneFlashInput[i] -= dt;

        if (gLaneFlashInput[i] < 0.0f)
            gLaneFlashInput[i] = 0.0f;
    }

    gSprite->Begin(D3DXSPRITE_ALPHABLEND);

    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);

    D3DSURFACE_DESC desc;
    gOverlayTex->GetLevelDesc(0, &desc);

    float scaleX = screenW / (float)desc.Width;
    float scaleY = screenH / (float)desc.Height;

    D3DXVECTOR2 scaling(scaleX, scaleY);
    D3DXVECTOR2 pos(0, 0);

    D3DXMATRIX mat;
    D3DXMatrixTransformation2D(
        &mat,
        nullptr,
        0,
        &scaling,
        nullptr,
        0,
        &pos
    );

    gSprite->SetTransform(&mat);

    gSprite->Draw(
        gOverlayTex,
        nullptr,
        nullptr,
        nullptr,
        D3DCOLOR_ARGB(195, 255, 255, 255)
    );

    float laneSpacing = 140.0f;
    float totalLaneWidth = laneSpacing * 5.0f;
    float startX = (screenW * 0.5f) - (totalLaneWidth * 0.5f);
    float landingY = screenH - 160.0f;

    gSprite->SetTransform(&identity);

    for (int i = 0; i < 6; i++)
    {
        if (!gArrowTexL[i]) continue;

        float x = startX + i * laneSpacing;
        float y = landingY;

        D3DSURFACE_DESC desc;
        gArrowTexL[i]->GetLevelDesc(0, &desc);

        float targetSize = 123.0f;
        float scale = targetSize / desc.Width;

        D3DXVECTOR2 scaling(scale, scale);
        D3DXVECTOR2 translation(x, y);

        D3DXMATRIX mat;
        D3DXMatrixTransformation2D(
            &mat,
            nullptr,
            0,
            &scaling,
            nullptr,
            0,
            &translation
        );

        gSprite->SetTransform(&mat);

        D3DXVECTOR3 center(desc.Width * 0.5f, desc.Height * 0.5f, 0);

        LPDIRECT3DTEXTURE9 texToUse = gArrowTexL[i];

        // kalau tombol lagi ditekan → pakai flash texture
        if (heldMask & (1 << i))
        {
            if (gArrowFlashTex[i])
                texToUse = gArrowFlashTex[i];
        }

        gSprite->Draw(
            texToUse,
            NULL,
            &center,
            NULL,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );
    }
    

    // SCROLLNOTE
    float pixelsPerSecond = 500.0f;
    float hitLineY = landingY;

    for (int i = 0; i < 16; i++)
    {
        int index = Rhythm::GetCurrentNoteIndex() + i;
        if (!Rhythm::IsNoteValid(index)) break;

        double noteTime = Rhythm::GetNoteTime(index);
        double timeDiff = noteTime - currentTime;

        if (Rhythm::IsNoteHit(index) &&
            Rhythm::GetNoteHitVisualTimer(index) <= 0.0f)
            continue;

        float noteY = hitLineY - (float)(timeDiff * pixelsPerSecond);

        if (noteY < -100) continue;
        if (noteY > screenH + 100) continue;

        int laneIndex = 0;
        int mask = Rhythm::GetNoteKeyMask(index);

        for (int k = 0; k < 6; k++)
            if (mask & (1 << k)) { laneIndex = k; break; }

        float x = startX + laneIndex * laneSpacing;

        D3DSURFACE_DESC desc;
        gArrowTex[laneIndex]->GetLevelDesc(0, &desc);

        float targetSize = 123.0f;
        float scale = targetSize / desc.Width;

        D3DXVECTOR2 scaling(scale, scale);
        D3DXVECTOR2 translation(x, noteY);

        D3DXMATRIX mat;
        D3DXMatrixTransformation2D(
            &mat,
            nullptr,
            0,
            &scaling,
            nullptr,
            0,
            &translation
        );

        gSprite->SetTransform(&mat);

        D3DXVECTOR3 center(desc.Width * 0.5f, desc.Height * 0.5f, 0);

        float flash = gNoteHitFlash[laneIndex];

        D3DCOLOR color;

        bool isFlash = false;

        LPDIRECT3DTEXTURE9 texToUse = gArrowTex[laneIndex];

        gSprite->Draw(
            texToUse,
            NULL,
            &center,
            NULL,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );
    }

    gSprite->SetTransform(&identity);

    int judgeIndex = -1;

    switch (Rhythm::GetLastJudgement())
    {
    case Rhythm::Judgement::Perfect: judgeIndex = 0; break;
    case Rhythm::Judgement::Good:    judgeIndex = 1; break;
    case Rhythm::Judgement::Bad:     judgeIndex = 2; break;
    case Rhythm::Judgement::Miss:    judgeIndex = 3; break;
    }

    if (judgeIndex >= 0 && gJudgeTex[judgeIndex])
    {
        D3DSURFACE_DESC desc;
        gJudgeTex[judgeIndex]->GetLevelDesc(0, &desc);

        float scale = 0.8f;

        D3DXVECTOR2 scaling(scale, scale);
        D3DXVECTOR2 translation(screenW * 0.5f, screenH * 0.18f);

        D3DXMATRIX mat;
        D3DXMatrixTransformation2D(
            &mat,
            nullptr,
            0,
            &scaling,
            nullptr,
            0,
            &translation
        );

        gSprite->SetTransform(&mat);

        D3DXVECTOR3 center(desc.Width / 2.0f, desc.Height / 2.0f, 0);

        gSprite->Draw(
            gJudgeTex[judgeIndex],
            NULL,
            &center,
            NULL,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );
    }


    //combo
    int combo = Rhythm::GetCombo();

    if (combo >= 1)
    {
        float digitScale = COMBO_SCALE;
        float xScale = COMBO_SCALE * COMBO_X_RATIO;

        std::string comboStr = "X" + std::to_string(combo);

        // ===== HITUNG TOTAL WIDTH =====
        float totalWidth = 0.0f;

        for (int i = 0; i < comboStr.size(); i++)
        {
            char c = comboStr[i];
            int texIndex = (c == 'X') ? 10 : (c - '0');

            if (!gDigitTex[texIndex]) continue;

            D3DSURFACE_DESC desc;
            gDigitTex[texIndex]->GetLevelDesc(0, &desc);

            float scale = (c == 'X') ? xScale : digitScale;
            totalWidth += desc.Width * scale;
        }

        // ===== CENTER POSITION =====
        float centerX = screenW * 0.5f;
        float baseY = screenH * 0.25f;

        float cursorX = centerX - totalWidth * 0.5f;

        // ===== DRAW DIGITS =====
        for (int i = 0; i < comboStr.size(); i++)
        {
            char c = comboStr[i];
            int texIndex = (c == 'X') ? 10 : (c - '0');

            if (!gDigitTex[texIndex]) continue;

            D3DSURFACE_DESC desc;
            gDigitTex[texIndex]->GetLevelDesc(0, &desc);

            float scale = (c == 'X') ? xScale : digitScale;

            D3DXVECTOR2 scaling(scale, scale);
            D3DXVECTOR2 translation(cursorX, baseY);

            // optional: naikkan X sedikit
            if (c == 'X')
                translation.y -= -1.0f;

            D3DXMATRIX mat;
            D3DXMatrixTransformation2D(
                &mat,
                nullptr,
                0,
                &scaling,
                nullptr,
                0,
                &translation
            );

            gSprite->SetTransform(&mat);

            D3DXVECTOR3 center(0, desc.Height * 0.5f, 0);

            gSprite->Draw(
                gDigitTex[texIndex],
                NULL,
                &center,
                NULL,
                D3DCOLOR_ARGB(255, 255, 255, 255)
            );

            if (c == 'X')
                cursorX += desc.Width * scale + 0.0f; // spacing khusus setelah X
            else
                cursorX += desc.Width * scale - 13.0f;  // spacing antar digi
        }
    }

    int score = Rhythm::GetScore();
    std::string scoreStr = std::to_string(score);

    while (scoreStr.length() < 7)
        scoreStr = "0" + scoreStr;

    float digitScale = SCORE_SCALE;

    float totalWidth = 0.0f;

    for (int i = 0; i < scoreStr.size(); i++)
    {
        int texIndex = scoreStr[i] - '0';

        if (!gDigitTex[texIndex]) continue;

        D3DSURFACE_DESC desc;
        gDigitTex[texIndex]->GetLevelDesc(0, &desc);

        totalWidth += desc.Width * digitScale;
    }

    float centerX = screenW * 0.5f;
    float baseY = screenH * 0.94f;

    float cursorX = centerX - totalWidth * 0.5f;

    for (int i = 0; i < scoreStr.size(); i++)
    {
        int texIndex = scoreStr[i] - '0';
        if (!gDigitTex[texIndex]) continue;

        D3DSURFACE_DESC desc;
        gDigitTex[texIndex]->GetLevelDesc(0, &desc);

        float scale = digitScale;

        D3DXVECTOR2 scaling(scale, scale);
        D3DXVECTOR2 translation(cursorX, baseY);

        D3DXMATRIX mat;
        D3DXMatrixTransformation2D(
            &mat,
            nullptr,
            0,
            &scaling,
            nullptr,
            0,
            &translation
        );

        gSprite->SetTransform(&mat);

        D3DXVECTOR3 center(0, desc.Height * 0.5f, 0);

        gSprite->Draw(
            gDigitTex[texIndex],
            NULL,
            &center,
            NULL,
            D3DCOLOR_ARGB(255, 255, 255, 255)
        );

        cursorX += desc.Width * scale;
    }

    gSprite->End();


    // 5️⃣ Score text (paling akhir)
    if (gFont)
    {
        
    }   
}

void Overlay::OnLostDevice()
{
    if (gSprite) gSprite->OnLostDevice();
    if (gFont) gFont->OnLostDevice();
    if (gCornerTex)
    {
        gCornerTex->Release();
        gCornerTex = nullptr;
    }
    for (int i = 0; i < 6; i++)
    {
        if (gArrowTex[i])
        {
            gArrowTex[i]->Release();
            gArrowTex[i] = nullptr;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (gJudgeTex[i])
        {
            gJudgeTex[i]->Release();
            gJudgeTex[i] = nullptr;
        }
    }

    for (int i = 0; i < 11; i++)
    {
        if (gDigitTex[i])
        {
            gDigitTex[i]->Release();
            gDigitTex[i] = nullptr;
        }
    }

    for (int i = 0; i < 6; i++)
    {
        if (gArrowFlashTex[i])
        {
            gArrowFlashTex[i]->Release();
            gArrowFlashTex[i] = nullptr;
        }
    }
}

void Overlay::OnResetDevice()
{
    if (gSprite) gSprite->OnResetDevice();
    if (gFont) gFont->OnResetDevice();
}
